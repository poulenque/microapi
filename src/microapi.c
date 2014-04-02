#include "microapi.h"

#include <stdio.h>

#define FALSE 0
#define TRUE 1

//TODO
//
// implement multiple window
// -> should glewInit be called once per new window ?
// event handling
// window resizing
// m_close should delete glPrograms

struct m_window{
	SDL_Window* sdl_window;
	SDL_GLContext context;
};

//list all windows
static m_window** windows;
static int window_count=0;
static int window_max=0;

static char* readFile(const char *path);
static void printShaderLog(GLuint shaderId);
static void printProgramLog(GLuint programId);

static void(*main_loop_function)(void);
static void(*typing_function)(unsigned char key, int x, int y);
static void(*keystate_function)(const Uint8* keystate,int x,int y);

void m_setWindowTitle(m_window* window,const char* title){
	SDL_SetWindowTitle(window->sdl_window,title);
}
void m_setPosition(m_window* window,int x, int y){
	SDL_SetWindowPosition(window->sdl_window, x, y);
}
void m_setSize(m_window* window,int w, int h){

}

void m_setMainLoopFunction(void(*f)(void)){
	main_loop_function=f;
}
void m_setTypingFunction(void(*f)(unsigned char key, int x, int y)){
	typing_function=f;
}
void m_setKeyStateFunction(void(*f)(const Uint8* keystate,int x,int y)){
	keystate_function = f;
}

static bool quit = FALSE;

void m_quitMainLoop(){
	quit=TRUE;
}

void m_MainLoop(){
	quit = FALSE;
	SDL_Event e;

	SDL_StartTextInput();
	while( !quit ) {
		while( SDL_PollEvent( &e ) != 0 ) {
			if( e.type == SDL_QUIT ) {
				quit = TRUE;
			} else if( e.type == SDL_TEXTINPUT ) {
				// if a function had been choosen
				if(typing_function){
					int x = 0, y = 0;
					SDL_GetMouseState( &x, &y );
					typing_function(e.text.text[0], x, y);
				}
			}
		}
		if(keystate_function){
			int x = 0, y = 0;
			SDL_GetMouseState( &x, &y );
			keystate_function(SDL_GetKeyboardState(NULL),x,y);
		}
		main_loop_function();
		// ==========
		// == TODO == this part will change with multiple window
		// ==========
		SDL_GL_SwapWindow( windows[window_count-1]->sdl_window );
		// ==========
		// ==========
	}
	SDL_StopTextInput();

}


bool m_init(int gl_major_version,int gl_minor_version){
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		return FALSE;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_version );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_version );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	return TRUE;	
}

void m_close(){
	//delete all windows
	for(int i=0;i<window_count;i++){
		SDL_DestroyWindow( windows[i]->sdl_window );
		windows[i]->sdl_window = NULL;
	}
	SDL_Quit();
}


m_window* m_newWindow(int width,int height,const char* title,bool resizable){

	if(window_count){
		printf("=======================================================\n");
		printf("== ERROR : for now only one window can be created... ==\n");
		printf("==         this is in the TODO list :¬P              ==\n");
		printf("==         have to figurate how to open more than    ==\n");
		printf("==         one opengl context ._.                    ==\n");
		printf("=======================================================\n");
		return NULL;
	}

	m_window* window = malloc(sizeof(m_window));
	if(title==NULL){title="";}
	Uint32 flags=SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;
	if(resizable){
		flags|=SDL_WINDOW_RESIZABLE;
	}
	window->sdl_window = SDL_CreateWindow( title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height,
		flags);
	if( window->sdl_window == NULL ) {
		printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
		return FALSE;
	}

	window->context = SDL_GL_CreateContext( window->sdl_window );
	if( window->context == NULL ) {
		printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		return FALSE;
	}

	// //Initialize GLEW
	// glewExperimental = GL_TRUE;
	// GLenum glewError = glewInit();
	// if(glewError!=GLEW_OK){printf("Error initializing GLEW! %s\n",glewGetErrorString(glewError));}

	//Use Vsync
	if(SDL_GL_SetSwapInterval(1)<0){printf("Warning: Unable to set VSync! SDL Error: %s\n",SDL_GetError());}

	//================================
	window_count++;
	if(window_count>window_max){
		window_max=2*(window_max+1);
		windows=realloc(windows,window_max*sizeof(m_window*));
	}
	windows[window_count-1]=window;
	//================================
	return window;
}

bool m_loadShader(GLuint* shaderId,GLuint shaderType,const char* path){
	//TODO : aller chercher la string qui correspond au code
	// const GLchar * vertexShaderSource[]={path};
	const GLchar * vertexShaderSource[]={readFile(path)};

	*shaderId = glCreateShader( shaderType );
	glShaderSource( *shaderId, 1, vertexShaderSource, NULL );
	glCompileShader( *shaderId );

	//==================================================
	//Check compilation
	GLint shaderCompiled = GL_FALSE;
	glGetShaderiv( *shaderId, GL_COMPILE_STATUS, &shaderCompiled );
	if( shaderCompiled != GL_TRUE ) {
		printf(
			"Unable to compile %s shader %d : %s!\n",
			shaderType==GL_VERTEX_SHADER?"vertex"
			:shaderType==GL_FRAGMENT_SHADER?"fragment":"geometry"
			,*shaderId
			,path );
		printShaderLog( *shaderId );
		return FALSE;
	}
	return TRUE;
}

bool m_generateGLProgram(GLuint* programId,GLuint vertexShaderId,GLuint geometryShaderId,GLuint fragmentShaderId){
	*programId = glCreateProgram();
	glAttachShader( *programId, vertexShaderId );
	glAttachShader( *programId, geometryShaderId );
	glAttachShader( *programId, fragmentShaderId );
	glLinkProgram( *programId );

	//Check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv( *programId, GL_LINK_STATUS, &programSuccess );
	if( programSuccess != GL_TRUE ) {
		printf( "Error linking program %d!\n", *programId );
		printProgramLog( *programId );
		return FALSE;
	}
}


static void printShaderLog( GLuint shader ) {
	if( glIsShader( shader ) ) {
		int infoLogLength = 0;
		int maxLength = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
		char* infoLog = malloc(sizeof(char)*maxLength);
		
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if(infoLogLength>0){printf( "%s\n", infoLog );}
		free(infoLog);
	} else {
		printf( "Name %d is not a shader\n", shader );
	}
}

static void printProgramLog( GLuint program ) {
	if( glIsProgram( program ) ) {
		int infoLogLength = 0;
		int maxLength = 0;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
		char* infoLog = malloc(sizeof(char)*maxLength);
		
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if(infoLogLength>0){printf( "%s\n", infoLog );}
		free(infoLog);
	} else {
		printf( "Name %d is not a program\n", program );
	}
}

static char* readFile(const char *path) {
	char *file_txt = NULL;
	int string_size,read_size;
	FILE *file = fopen(path,"r");
	if (file) {
		//seek the last byte of the file
		fseek(file,0,SEEK_END);
		//offset from the first to the last byte, or in other words, filesize
		string_size = ftell(file);
		//go back to the start of the file
		rewind(file);

		file_txt = (char*) malloc (sizeof(char)*(string_size + 1) );
		//read it all in one operation
		read_size = fread(file_txt,sizeof(char),string_size,file);
		//fread doesnt set it so put a \0 in the last position
		//and buffer is now officialy a string
		// buffer[string_size+1] = '\0';
		file_txt[string_size] = '\0';

		if (string_size != read_size) {
			//something went wrong, throw away the memory and set
			//the buffer to NULL
			free(file_txt);
			file_txt = "";
		}
	}else{
		file_txt = "";
	}
	return file_txt;
}

