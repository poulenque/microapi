#ifndef MICROAPI__H
#define MICROAPI__H

#include <SDL2/SDL.h>
// #include <GLES3/gl3.h>
#include <GL/glew.h>
#include <GL/gl.h>
// #include <GL/glu.h>
// #include <SDL2/SDL_opengl.h>

typedef int Bool;

typedef struct m_window m_window;

struct m_window{
	SDL_Window* sdl_window;
	SDL_GLContext context;
	float aspect_ratio;
};

typedef float m_vec1[1];
typedef float m_vec2[2];
typedef float m_vec3[3];
typedef float m_vec4[4];
typedef float m_mat1[1];
typedef float m_mat2[4];
typedef float m_mat3[9];
typedef float m_mat4[16];

//m_init(3,2) will create a 3.2 opengl context
Bool m_init(int gl_major_version,int gl_minor_version);
void m_close();

m_window* m_newWindow(int width,int height,const char* title,Bool resizable);
void m_setWindowTitle(m_window* window,const char* title);
void m_setPosition(m_window* window,int x, int y);
void m_setSize(m_window* window,int w, int h);

void m_setTypingFunction(void(*f)(unsigned char key, int x, int y,int leftButton,int rightButton, int middleButton, int scrollUp, int scrollDown));
void m_setKeyStateFunction(void(*f)(const Uint8* keystate,int x, int y,int leftButton,int rightButton, int middleButton, int scrollUp, int scrollDown));
void m_setMainLoopFunction(void(*f)(void));
void m_MainLoop();

void m_quitMainLoop();

// =================
// == OPENGL TOOL ==
// =================
Bool m_loadShader(GLuint* shaderId,GLuint shaderType,const char* path);
Bool m_generateGLProgram(GLuint* programId,GLuint vertexShaderId,GLuint geometryShaderId,GLuint fragmentShaderId);

Bool m_mat1_inverse(m_mat1 mat_in,m_mat1 mat_out);
Bool m_mat2_inverse(m_mat2 mat_in,m_mat2 mat_out);
Bool m_mat3_inverse(m_mat3 mat_in,m_mat3 mat_out);
Bool m_mat4_inverse(m_mat4 mat_in,m_mat4 mat_out);

void m_mat2_translation(m_mat2 mat,m_vec1 v);
void m_mat3_translation(m_mat3 mat,m_vec2 v);
void m_mat4_translation(m_mat4 mat,m_vec3 v);

void m_mat3_rotation(m_mat3 mat,float theta);
void m_mat4_rotation(m_mat4 mat,m_vec3 v);

void m_mat2_scale(m_mat2 mat,m_vec1 v);
void m_mat3_scale(m_mat3 mat,m_vec2 v);
void m_mat4_scale(m_mat4 mat,m_vec3 v);

void m_mat1_mult(m_mat1 mat,m_mat1 m1,m_mat1 m2);
void m_mat2_mult(m_mat2 mat,m_mat2 m1,m_mat2 m2);
void m_mat3_mult(m_mat3 mat,m_mat3 m1,m_mat3 m2);
void m_mat4_mult(m_mat4 mat,m_mat4 m1,m_mat4 m2);
// ========
// ==TODO==
// ========

// void m_mat1_add(m_mat1 mat,m_mat1 m1,m_mat1 m2);
// void m_mat2_add(m_mat2 mat,m_mat2 m1,m_mat2 m2);
// void m_mat3_add(m_mat3 mat,m_mat3 m1,m_mat3 m2);
// void m_mat4_add(m_mat4 mat,m_mat4 m1,m_mat4 m2);

// void m_mat1_sub(m_mat1 mat,m_mat1 m1,m_mat1 m2);
// void m_mat2_sub(m_mat2 mat,m_mat2 m1,m_mat2 m2);
// void m_mat3_sub(m_mat3 mat,m_mat3 m1,m_mat3 m2);
// void m_mat4_sub(m_mat4 mat,m_mat4 m1,m_mat4 m2);



void m_matLookAt(m_mat4 mat,m_vec3 eye,m_vec3 center,m_vec3 up);
void m_matProjection(m_mat4 mat,float fovy,float aspect_ratio,float near,float far);
// =================
// =================


#endif