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
// resize event set aspect_ratio to the correct value

struct m_window{
	SDL_Window* sdl_window;
	SDL_GLContext context;
	float aspect_ratio;
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
	//TODO
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

static Bool quit = FALSE;

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


Bool m_init(int gl_major_version,int gl_minor_version){
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


m_window* m_newWindow(int width,int height,const char* title,Bool resizable){
	aspect_ratio=((float)width)/height;

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

Bool m_loadShader(GLuint* shaderId,GLuint shaderType,const char* path){
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

Bool m_generateGLProgram(GLuint* programId,GLuint vertexShaderId,GLuint geometryShaderId,GLuint fragmentShaderId){
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






















Bool m_mat1_inverse(m_mat1 mat_in,m_mat1 mat_out){
	mat_out[0]=1./mat_in[0];
}
Bool m_mat2_inverse(m_mat2 mat_in,m_mat2 mat_out){
	float det_invert=1./(mat_in[0]*mat_in[3]-mat_in[1]*mat_in[2]) 
	mat_out[0] =  det_invert*mat_in[3];
	mat_out[1] = -det_invert*mat_in[1];
	
	mat_out[2] = -det_invert*mat_in[2];
	mat_out[3] =  det_invert*mat_in[0];
}
Bool m_mat3_inverse(m_mat3 mat_in,m_mat3 mat_out){
#define a__ mat_in[0]
#define b__ mat_in[1]
#define c__ mat_in[2]
#define d__ mat_in[3]
#define e__ mat_in[4]
#define f__ mat_in[5]
#define g__ mat_in[6]
#define h__ mat_in[7]
#define i__ mat_in[8]
#define A= e__*i__ - f__*h__;
#define B= f__*g__ - d__*i__;
#define C= d__*h__ - e__-g__;
#define D= c__*h__ - b__*i__;
#define E= a__*i__ - c__*g__;
#define F= b__*g__ - a__*h__;
#define G= b__*f__ - c__*e__;
#define H= c__*d__ - a__*f__;
#define I= a__*e__ - b__*d__;
	float det_invert =
		 mat_in[0]*(mat_in[4]*mat_in[8]-mat_in[5]*mat_in[7])
		-mat_in[1]*(mat_in[8]*mat_in[3]-mat_in[5]*mat_in[6])
		+mat_in[2]*(mat_in[3]*mat_in[7]-mat_in[4]*mat_in[6]);
	det_invert=1./det_invert;


	mat_out[0] = A;
	mat_out[1] = D;
	mat_out[2] = G;

	mat_out[3] = B;
	mat_out[4] = E;
	mat_out[5] = H;
	
	mat_out[6] = C;
	mat_out[7] = F;
	mat_out[8] = I;
#undef a__
#undef b__
#undef c__
#undef d__
#undef e__
#undef f__
#undef g__
#undef h__
#undef i__
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
#undef I
}
Bool m_mat4_inverse(m_mat4 mat_in,m_mat4 mat_out){
#define a__ mat_in[0]
#define b__ mat_in[1]
#define c__ mat_in[2]
#define d__ mat_in[3]
#define e__ mat_in[4]
#define f__ mat_in[5]
#define g__ mat_in[6]
#define h__ mat_in[7]
#define i__ mat_in[8]
#define j__ mat_in[9]
#define k__ mat_in[10]
#define l__ mat_in[11]
#define m__ mat_in[12]
#define n__ mat_in[13]
#define o__ mat_in[14]
#define p__ mat_in[15]
	float det_invert=9000000000000;

	mat_out[0] = det_invert*;
	mat_out[1] = det_invert*;
	mat_out[2] = det_invert*;
	mat_out[3] = det_invert*;

	mat_out[4] = det_invert*;
	mat_out[5] = det_invert*;
	mat_out[6] = det_invert*;
	mat_out[7] = det_invert*;

	mat_out[8] = det_invert*;
	mat_out[9] = det_invert*;
	mat_out[10]= det_invert*;
	mat_out[11]= det_invert*;

	mat_out[12]= det_invert*;
	mat_out[13]= det_invert*;
	mat_out[14]= det_invert*;
	mat_out[15]= det_invert*;
#undef a__
#undef b__
#undef c__
#undef d__
#undef e__
#undef f__
#undef g__
#undef h__
#undef i__
#undef j__
#undef k__
#undef l__
#undef m__
#undef n__
#undef o__
#undef p__
}

void m_matLookAt(m_mat4 mat,m_vec4 eye,m_vec4 center,m_vec4 up){

}
void m_matProjection(m_mat4 mat,float f,float near,float far){
	mat[0] = 1;
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = 0;

	mat[4] = 0;
	mat[5] = windows[0]->aspect_ratio;
	mat[6] = 0;
	mat[7] = 0;

	mat[8] = 0;
	mat[9] = 0;
	mat[10]= (far+near)/(near-far);
	mat[11]= 2*far*near/(near-far);

	mat[12]= 0;
	mat[13]= 0;
	mat[14]= -1/f;
	mat[15]= 0;
}

//--TRANSLATE--TRANSLATE--TRANSLATE--TRANSLATE--TRANSLATE
void m_mat2_translation(m_mat2 mat,m_vec1 v){
	mat[0] = 0;
	mat[1] = v[0];
	mat[2] = 0;
	mat[3] = 1;
}
void m_mat3_translation(m_mat3 mat,m_vec2 v){
	mat[0] = 0;
	mat[1] = 0;
	mat[2] = v[0];

	mat[3] = 0;
	mat[4] = 0;
	mat[5] = v[1];

	mat[6] = 0;
	mat[7] = 0;
	mat[8] = 1;
}
void m_mat4_translation(m_mat4 mat,m_vec3 v){
	mat[0] = 0;
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = v[0];

	mat[4] = 0;
	mat[5] = 0;
	mat[6] = 0;
	mat[7] = v[1];

	mat[8] = 0;
	mat[9] = 0;
	mat[10]= 0;
	mat[11]= v[2];

	mat[12]= 0;
	mat[13]= 0;
	mat[14]= 0;
	mat[15]= 1;
}
//--ROTATE--ROTATE--ROTATE--ROTATE--ROTATE--ROTATE
// void m_mat2_rotation(m_mat2 mat,m_vec1 v){
// 	float theta = sqrt(v[0]*v[0]);
// 	theta=theta>=0?1:-1;
// 	mat[0] = theta;
// 	mat[1] = 0;
// 	mat[2] = 1;
// 	mat[3] = 0;
// }
void m_mat3_rotation(m_mat3 mat,float theta){
	// float theta = sqrt(v[0]*v[0]+v[1]*v[1]);
	float c=cos(theta);
	float s=sin(theta);

	mat[0] = c;
	mat[1] = -s;
	mat[2] = 0;

	mat[3] = s;
	mat[4] = c;
	mat[5] = 0;

	mat[6] = 0;
	mat[7] = 0;
	mat[8] = 1;
}
void m_mat4_rotation(m_mat4 mat,m_vec3 v){
	float theta = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	float c=cos(theta);
	float s=sin(theta);
	float inv_theta=1./theta;
	float x=v[0]*inv_theta;
	float y=v[1]*inv_theta;
	float z=v[2]*inv_theta;

	mat[0] = 0;
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = 0;

	mat[4] = 0;
	mat[5] = 0;
	mat[6] = 0;
	mat[7] = 0;

	mat[8] = 0;
	mat[9] = 0;
	mat[10]= 0;
	mat[11]= 0;

	mat[12]= 0;
	mat[13]= 0;
	mat[14]= 0;
	mat[15]= 0;
}
//--SCALE--SCALE--SCALE--SCALE--SCALE--SCALE
void m_mat2_scale(m_mat2 mat,m_vec1 v){
	mat[0] = v[0];
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = 0;
}
void m_mat3_scale(m_mat3 mat,m_vec2 v){
	mat[0] = v[0];
	mat[1] = 0;
	mat[2] = 0;

	mat[3] = 0;
	mat[4] = v[1];
	mat[5] = 0;

	mat[6] = 0;
	mat[7] = 0;
	mat[8] = 1;
}
void m_mat4_scale(m_mat4 mat,m_vec3 v){
	mat[0] = v[0];
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = 0;

	mat[4] = 0;
	mat[5] = v[1];
	mat[6] = 0;
	mat[7] = 0;

	mat[8] = 0;
	mat[9] = 0;
	mat[10]= v[2];
	mat[11]= 0;

	mat[12]= 0;
	mat[13]= 0;
	mat[14]= 0;
	mat[15]= 1;
}
