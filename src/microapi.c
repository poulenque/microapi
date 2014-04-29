#include "microapi.h"

#include <stdio.h>

#define FALSE 0
#define TRUE 1

//TODO
//
// window resizing
//   wtf with the content ??
//
// implement multiple window
// -> should glewInit be called once per new window ?
// -> should m_close delete glPrograms ?
//


//list all windows
static m_window** windows;
static int window_count=0;
static int window_max=0;

static char* readFile(const char *path);
static void printShaderLog(GLuint shaderId);
static void printProgramLog(GLuint programId);

static void(*main_loop_function)(void);
static void(*typing_function)(unsigned char key, int x, int y,int leftButton,int rightButton, int middleButton, int scrollUp, int scrollDown);
static void(*keystate_function)(const Uint8* keystate,int x,int y,int leftButton,int rightButton, int middleButton, int scrollUp, int scrollDown);

void m_setWindowTitle(m_window* window,const char* title){
	SDL_SetWindowTitle(window->sdl_window,title);
}
void m_setPosition(m_window* window,int x, int y){
	SDL_SetWindowPosition(window->sdl_window, x, y);
}
void m_setSize(m_window* window,int w, int h){
	SDL_SetWindowSize(window->sdl_window,w,h);
	window->aspect_ratio=((float)w)/h;
	glViewport(0,0,w,h);
}

void m_setMainLoopFunction(void(*f)(void)){
	main_loop_function=f;
}
void m_setTypingFunction(void(*f)(unsigned char key, int x, int y,int leftButton,int rightButton, int middleButton, int scrollUp, int scrollDown)){
	typing_function=f;
}
void m_setKeyStateFunction(void(*f)(const Uint8* keystate,int x,int y,int leftButton,int rightButton, int middleButton, int scrollUp, int scrollDown)){
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
			} else if( e.type == SDL_WINDOWEVENT){
				if(e.window.event==SDL_WINDOWEVENT_RESIZED){
					int w=e.window.data1;
					int h=e.window.data2;
					windows[0]->aspect_ratio=((float)w)/h;
					// printf("Window %d resized to %dx%d\n",e.window.windowID, e.window.data1,e.window.data2);
					//TODO
					// is that deprecated ??
					glViewport(0,0,w,h);
				}
			} else if( e.type == SDL_TEXTINPUT ) {
				// if a function had been choosen
				if(typing_function){
					int x = 0, y = 0;
					Uint8 mouseState = SDL_GetMouseState( &x, &y );
					int leftButton = mouseState&SDL_BUTTON(1);
					int rightButton= mouseState&SDL_BUTTON(3);
					int middleButton=mouseState&SDL_BUTTON(2);
					int scrollUp  =  mouseState&SDL_BUTTON(4);
					int scrollDown = mouseState&SDL_BUTTON(5);
					typing_function(e.text.text[0], x, y,leftButton,rightButton, middleButton, scrollUp, scrollDown);
				}
			}
		}
		if(keystate_function){
			int x = 0, y = 0;
			Uint8 mouseState = SDL_GetMouseState( &x, &y );
			int leftButton = mouseState&SDL_BUTTON(1);
			int rightButton= mouseState&SDL_BUTTON(3);
			int middleButton=mouseState&SDL_BUTTON(2);
			int scrollUp  =  mouseState&SDL_BUTTON(4);
			int scrollDown = mouseState&SDL_BUTTON(5);
			keystate_function(SDL_GetKeyboardState(NULL),x,y,leftButton,rightButton, middleButton, scrollUp, scrollDown);
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
	window->aspect_ratio=((float)width)/height;
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
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if(glewError!=GLEW_OK){printf("Error initializing GLEW! %s\n",glewGetErrorString(glewError));}

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
	if(mat_in[0]==0)return FALSE;
	mat_out[0]=1./mat_in[0];
	return TRUE;
}
Bool m_mat2_inverse(m_mat2 mat_in,m_mat2 mat_out){
	float det_invert=(mat_in[0]*mat_in[3]-mat_in[1]*mat_in[2]);
	if(det_invert==0)return FALSE;
	det_invert=1./det_invert;
	mat_out[0] =  det_invert*mat_in[3];
	mat_out[1] = -det_invert*mat_in[1];
	
	mat_out[2] = -det_invert*mat_in[2];
	mat_out[3] =  det_invert*mat_in[0];
	return TRUE;
}
#define A mat_in[0]
#define B mat_in[1]
#define C mat_in[2]
#define D mat_in[3]
#define E mat_in[4]
#define F mat_in[5]
#define G mat_in[6]
#define H mat_in[7]
#define I mat_in[8]
#define J mat_in[9]
#define K mat_in[10]
#define L mat_in[11]
#define M mat_in[12]
#define N mat_in[13]
#define O mat_in[14]
#define P mat_in[15]
Bool m_mat3_inverse(m_mat3 mat_in,m_mat3 mat_out){

	float det_invert =
		 mat_in[0]*(mat_in[4]*mat_in[8]-mat_in[5]*mat_in[7])
		-mat_in[1]*(mat_in[8]*mat_in[3]-mat_in[5]*mat_in[6])
		+mat_in[2]*(mat_in[3]*mat_in[7]-mat_in[4]*mat_in[6]);
	if(det_invert==0)return FALSE;
	det_invert=1./det_invert;

	mat_out[0] = E*I - F*H;
	mat_out[1] = C*H - B*I;
	mat_out[2] = B*F - C*E;

	mat_out[3] = F*G - D*I;
	mat_out[4] = A*I - C*G;
	mat_out[5] = C*D - A*F;
	
	mat_out[6] = D*H - E-G;
	mat_out[7] = B*G - A*H;
	mat_out[8] = A*E - B*D;
	return TRUE;
}
Bool m_mat4_inverse(m_mat4 mat_in,m_mat4 mat_out){
	float det_invert =
	  A*( F*K*P - F*L*O - G*J*P + G*L*N + H*J*O - H*K*N)
	+ B*(-E*K*P + E*L*O + G*I*P - G*L*M - H*I*O + H*K*M)
	+ C*( E*J*P - E*L*N - F*I*P + F*L*M + H*I*N + H*J*M)
	+ D*(-E*J*O + E*K*N + F*I*O - F*K*M - G*I*N + G*J*M);
	if(det_invert==0)return FALSE;
	det_invert=1./det_invert;

	mat_out[0] = det_invert*( -H*K*N + G*L*N + H*J*O - F*L*O - G*J*P + F*K*P);
	mat_out[1] = det_invert*(  D*K*N - C*L*N - D*J*O + B*L*O + C*J*P - B*K*P);
	mat_out[2] = det_invert*( -D*G*M + C*H*N + D*F*O - B*H*O - C*F*P + B*G*P);
	mat_out[3] = det_invert*(  D*G*J - C*H*J - D*F*K + B*H*K + C*F*L - B*G*L);

	mat_out[4] = det_invert*(  H*K*M - G*L*M - H*I*O + E*L*O + G*I*P - E*K*P);
	mat_out[5] = det_invert*( -D*K*M + C*L*M + D*I*O - A*L*O - C*I*P + A*K*P);
	mat_out[6] = det_invert*(  D*G*M - C*H*M - D*E*O + A*H*O + C*E*P - A*G*P);
	mat_out[7] = det_invert*( -D*G*I + C*H*I + D*E*K - A*H*K - C*E*L + A*G*L);

	mat_out[8] = det_invert*( -H*J*M + F*L*M + H*I*N - E*L*N - F*I*P + E*J*P);
	mat_out[9] = det_invert*(  D*J*M - B*L*M - D*I*N + A*L*N + B*I*P - A*J*P);
	mat_out[10]= det_invert*( -D*F*M + B*H*M + D*E*N - A*H*N - B*E*P + A*F*P);
	mat_out[11]= det_invert*(  D*F*I - B*H*I - D*E*J + A*H*J + B*E*L - A*F*L);

	mat_out[12]= det_invert*(  G*J*M - F*K*M - G*I*N + E*K*N + F*I*O - E*J*O);
	mat_out[13]= det_invert*( -C*J*M + B*K*M + C*I*N - A*K*N - B*I*O + A*J*O);
	mat_out[14]= det_invert*(  C*F*M - B*G*M - C*E*N + A*G*N + B*E*O - A*F*O);
	mat_out[15]= det_invert*( -C*F*I + B*G*I + C*E*J - A*G*J - B*E*K + A*F*K);
	return TRUE;
}
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
#undef I
#undef J
#undef K
#undef L
#undef M
#undef N
#undef O
#undef P
//--TRANSLATE--TRANSLATE--TRANSLATE--TRANSLATE--TRANSLATE
//--TRANSLATE--TRANSLATE--TRANSLATE--TRANSLATE--TRANSLATE
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
//--ROTATE--ROTATE--ROTATE--ROTATE--ROTATE--ROTATE
//--ROTATE--ROTATE--ROTATE--ROTATE--ROTATE--ROTATE
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

	mat[0] = c         + x*x*(1-c);
	mat[1] = x*y*(1-c) - z*s;
	mat[2] = x*z*(1-c) + y*s;
	mat[3] = 0;

	mat[4] = y*x*(1-c) + z*s;
	mat[5] = c         + y*y*(1-c);
	mat[6] = y*z*(1-c) - x*s;
	mat[7] = 0;

	mat[8] = z*x*(1-c) - y*s;
	mat[9] = z*y*(1-c) + x*s;
	mat[10]= c         + z*z*(1-c);
	mat[11]= 0;

	mat[12]= 0;
	mat[13]= 0;
	mat[14]= 0;
	mat[15]= 1;
}
//--SCALE--SCALE--SCALE--SCALE--SCALE--SCALE
//--SCALE--SCALE--SCALE--SCALE--SCALE--SCALE
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
//--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC
//--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC
//--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC--CALC
void m_mat1_mult(m_mat1 mat,m_mat1 m1,m_mat1 m2){
	mat[0]=m1[0]*m2[0];
}
void m_mat2_mult(m_mat2 mat,m_mat2 m1,m_mat2 m2){
	mat[0]=m1[0]*m2[0] + m1[1]*m2[1];
	mat[1]=m1[0]*m2[2] + m1[1]*m2[3];
	mat[2]=m1[2]*m2[0] + m1[3]*m2[1];
	mat[3]=m1[2]*m2[2] + m1[3]*m2[3];
}
void m_mat3_mult(m_mat3 mat,m_mat3 m1,m_mat3 m2){
	mat[ 0]=m1[0]*m2[0] + m1[1]*m2[1] + m1[2]*m2[2];
	mat[ 1]=m1[0]*m2[3] + m1[1]*m2[4] + m1[2]*m2[5];
	mat[ 2]=m1[0]*m2[6] + m1[1]*m2[7] + m1[2]*m2[8];
	mat[ 3]=m1[3]*m2[0] + m1[4]*m2[1] + m1[5]*m2[2];
	mat[ 4]=m1[3]*m2[3] + m1[4]*m2[4] + m1[5]*m2[5];
	mat[ 5]=m1[3]*m2[6] + m1[4]*m2[7] + m1[5]*m2[8];
	mat[ 6]=m1[6]*m2[0] + m1[7]*m2[1] + m1[8]*m2[2];
	mat[ 7]=m1[6]*m2[3] + m1[7]*m2[4] + m1[8]*m2[5];
	mat[ 8]=m1[6]*m2[6] + m1[7]*m2[7] + m1[8]*m2[8];
}
void m_mat4_mult(m_mat4 mat,m_mat4 m1,m_mat4 m2){
	mat[ 0]=m1[ 0]*m2[ 0] + m1[ 1]*m2[ 1] + m1[ 2]*m2[ 2] + m1[ 3]*m2[ 3];
	mat[ 1]=m1[ 0]*m2[ 4] + m1[ 1]*m2[ 5] + m1[ 2]*m2[ 6] + m1[ 3]*m2[ 7];
	mat[ 2]=m1[ 0]*m2[ 8] + m1[ 1]*m2[ 9] + m1[ 2]*m2[10] + m1[ 3]*m2[11];
	mat[ 3]=m1[ 0]*m2[12] + m1[ 1]*m2[13] + m1[ 2]*m2[14] + m1[ 3]*m2[15];
	mat[ 4]=m1[ 4]*m2[ 0] + m1[ 5]*m2[ 1] + m1[ 6]*m2[ 2] + m1[ 7]*m2[ 3];
	mat[ 5]=m1[ 4]*m2[ 4] + m1[ 5]*m2[ 5] + m1[ 6]*m2[ 6] + m1[ 7]*m2[ 7];
	mat[ 6]=m1[ 4]*m2[ 8] + m1[ 5]*m2[ 9] + m1[ 6]*m2[10] + m1[ 7]*m2[11];
	mat[ 7]=m1[ 4]*m2[12] + m1[ 5]*m2[13] + m1[ 6]*m2[14] + m1[ 7]*m2[15];
	mat[ 8]=m1[ 8]*m2[ 0] + m1[ 9]*m2[ 1] + m1[10]*m2[ 2] + m1[11]*m2[ 3];
	mat[ 9]=m1[ 8]*m2[ 4] + m1[ 9]*m2[ 5] + m1[10]*m2[ 6] + m1[11]*m2[ 7];
	mat[10]=m1[ 8]*m2[ 8] + m1[ 9]*m2[ 9] + m1[10]*m2[10] + m1[11]*m2[11];
	mat[11]=m1[ 8]*m2[12] + m1[ 9]*m2[13] + m1[10]*m2[14] + m1[11]*m2[15];
	mat[12]=m1[12]*m2[ 0] + m1[13]*m2[ 1] + m1[14]*m2[ 2] + m1[15]*m2[ 3];
	mat[13]=m1[12]*m2[ 4] + m1[13]*m2[ 5] + m1[14]*m2[ 6] + m1[15]*m2[ 7];
	mat[14]=m1[12]*m2[ 8] + m1[13]*m2[ 9] + m1[14]*m2[10] + m1[15]*m2[11];
	mat[15]=m1[12]*m2[12] + m1[13]*m2[13] + m1[14]*m2[14] + m1[15]*m2[15];
}
//--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER
//--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER
//--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER--OTHER
void m_matLookAt(m_mat4 mat,m_vec3 eye,m_vec3 center,m_vec3 up){

}
void m_matProjection(m_mat4 mat,float fovy,float aspect_ratio,float near,float far){
	// mat[0] = 1;
	// mat[1] = 0;
	// mat[2] = 0;
	// mat[3] = 0;

	// mat[4] = 0;
	// mat[5] = windows[0]->aspect_ratio;
	// mat[6] = 0;
	// mat[7] = 0;

	// mat[8] = 0;
	// mat[9] = 0;
	// mat[10]= (far+near)/(near-far);
	// mat[11]= 2*far*near/(near-far);

	// mat[12]= 0;
	// mat[13]= 0;
	// mat[14]= -1/f;
	// mat[15]= 0;
}
