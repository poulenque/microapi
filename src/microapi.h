#ifndef MICROAPI__H
#define MICROAPI__H

#include <SDL2/SDL.h>
// #include <GLES3/gl3.h>
// #include <GL/glew.h>
#include <GL/gl.h>
// #include <GL/glu.h>
// #include <SDL2/SDL_opengl.h>

typedef int bool;

typedef struct m_window m_window;

typedef double m_vec1[1];
typedef double m_vec2[2];
typedef double m_vec3[3];
typedef double m_vec4[4];
typedef double m_mat1[1];
typedef double m_mat2[4];
typedef double m_mat3[9];
typedef double m_mat4[16];

//m_init(3,2) will create a 3.2 opengl context
bool m_init(int gl_major_version,int gl_minor_version);
void m_close();

m_window* m_newWindow(int width,int height,const char* title,bool resizable);
void m_setWindowTitle(m_window* window,const char* title);

// ========
// ==TODO==
// ========
void m_setPosition(m_window* window,int x, int y);
void m_setSize(m_window* window,int w, int h);
// ========
// ========

void m_setTypingFunction(void(*f)(unsigned char key, int x, int y));
void m_setKeyStateFunction(void(*f)(const Uint8* keystate,int x, int y));
void m_setMainLoopFunction(void(*f)(void));
void m_MainLoop();

void m_quitMainLoop();

// =================
// == OPENGL TOOL ==
// =================
bool m_loadShader(GLuint* shaderId,GLuint shaderType,const char* path);
bool m_generateGLProgram(GLuint* programId,GLuint vertexShaderId,GLuint geometryShaderId,GLuint fragmentShaderId);
// ========
// ==TODO==
// ========
// bool m_mat1_inverse(m_mat1 matrix);
// bool m_mat2_inverse(m_mat2 matrix);
// bool m_mat3_inverse(m_mat3 matrix);
// bool m_mat4_inverse(m_mat4 matrix);

// m_mat1 m_mat1_translation(m_vec1 v);
// m_mat2 m_mat2_translation(m_vec2 v);
// m_mat3 m_mat3_translation(m_vec3 v);
// m_mat4 m_mat4_translation(m_vec4 v);

// m_mat1 m_mat1_rotation(m_vec1 v);
// m_mat2 m_mat2_rotation(m_vec2 v);
// m_mat3 m_mat3_rotation(m_vec3 v);
// m_mat4 m_mat4_rotation(m_vec4 v);

// m_mat1 m_mat1_scale(m_vec1 v);
// m_mat2 m_mat2_scale(m_vec2 v);
// m_mat3 m_mat3_scale(m_vec3 v);
// m_mat4 m_mat4_scale(m_vec4 v);

// m_mat4 m_matLookAt();
// m_mat4 m_matProjection();
// =================
// =================


#endif