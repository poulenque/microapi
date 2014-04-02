#include "microapi.h"

#define FALSE 0
#define TRUE 1

bool gRenderQuad = TRUE;

GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;

bool initGL() {
	GLuint vertexShader;
	GLuint fragmentShader;
	if(!m_loadShader(&vertexShader,GL_VERTEX_SHADER,"src/vshader.glsl"))return FALSE;
	if(!m_loadShader(&fragmentShader,GL_FRAGMENT_SHADER,"src/fshader.glsl"))return FALSE;
	//==================================================
	if(!m_generateGLProgram(&gProgramID,vertexShader,0,fragmentShader))return FALSE;
	//==================================================
	//==================================================
	gVertexPos2DLocation = glGetAttribLocation( gProgramID, "position" );
	glClearColor( 0.f, 0.f, 0.f, 1.f );
	//VBO data
	GLfloat vertexData[] = {
		-0.5f, -0.5f, 0.0f ,
		 0.5f, -0.5f, 0.0f ,
		 0.5f,  0.5f, 0.0f ,
		-0.5f,  0.5f, 0.0f ,
	};
	//IBO data
	GLuint indexData[] = { 0, 1, 2, 3 };
	//Create VBO
	glGenBuffers( 1, &gVBO );
	glBindBuffer( GL_ARRAY_BUFFER, gVBO );
	glBufferData( GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW );
	//Create IBO
	glGenBuffers( 1, &gIBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indexData, GL_STATIC_DRAW );
	return TRUE;
}

void handleKeys( unsigned char key, int x, int y ) {
	//Toggle quad
	gRenderQuad = !gRenderQuad;
	if( key == 'q' ) {
		m_quitMainLoop();
	}
}

void keystate_function(const Uint8* keystate,int x, int y){
	if(keystate[SDL_SCANCODE_A]){
		printf("caca\n");
	}
}

static m_window* gWindow;

static float t =0;

void render() {
	t+=0.1;
	m_setPosition(gWindow,100+100*cos(t), 100+100*sin(t));

	glClear( GL_COLOR_BUFFER_BIT );
	if( gRenderQuad ) {
		glUseProgram( gProgramID );

		glEnableVertexAttribArray( gVertexPos2DLocation );

		glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		glVertexAttribPointer( gVertexPos2DLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
		glDrawElements( GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL );

		glDisableVertexAttribArray( gVertexPos2DLocation );

		glUseProgram(0);
	}
}

int main(int argc,char* args[]) {

	// if(!m_init(3,1)) {return 1;}
	if(!m_init(3,0)) {return 1;}

	gWindow = m_newWindow(640,480,"hello",FALSE);
	
	initGL();

	m_setTypingFunction(handleKeys);
	m_setKeyStateFunction(keystate_function);
	m_setMainLoopFunction(render);

	m_MainLoop();

	//Deallocate program
	glDeleteProgram( gProgramID );
	m_close();

	return 0;
}
