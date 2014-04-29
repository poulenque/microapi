#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h> // before glfw !
#include <GL/glfw.h>
#include <GL/gl.h>


void display(){
	printf("caca\n");
}


int newWindow(const char* title, int width, int height){
	/// Hint GLFW that we would like an OpenGL 3 context (at least)
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/// Hint for multisampling
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);

	/// Attempt to open the window: fails if required version unavailable
	/// @note Intel GPUs do not support OpenGL 3.0
	if( !glfwOpenWindow(width, height, 0,0,0,0, 32,0, GLFW_WINDOW ) ){
		fprintf( stderr, "Failed to open OpenGL 3 GLFW window.\n" );
		glfwTerminate();
		return EXIT_FAILURE;
	}

	/// Outputs the OpenGL version
	int major, minor, revision;
	glfwGetGLVersion(&major, &minor,&revision);
	printf("Opened GLFW OpenGL %i.%i.%i\n",major,minor,revision);

	// GLEW Initialization (must have a context)
	glewExperimental = 1;
	if( glewInit() != GLEW_NO_ERROR ){
		fprintf( stderr, "Failed to initialize GLEW\n"); 
		return EXIT_FAILURE;
	}

	/// Set window title
	glfwSetWindowTitle(title);

	return EXIT_SUCCESS;
}

static void (*_display)(void) = NULL;


void glfwMainLoop(){
	// assert(_display!=NULL);

	/// Render loop & keyboard input
	while(glfwGetKey(GLFW_KEY_ESC)!=GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED)){
		_display();
		glfwSwapBuffers();
	}

	/// Close OpenGL window and terminate GLFW
	glfwTerminate();
}



void glfwDisplayFunc(void (*display)(void)){    
    _display = display;
}














int main(int argc, char *argv[]) {

	if( !glfwInit() ){
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return 1;
	}    


	newWindow("hw4: Spiral",640,480);
	glfwDisplayFunc(display);

	glfwMainLoop();
	return 0;
}
