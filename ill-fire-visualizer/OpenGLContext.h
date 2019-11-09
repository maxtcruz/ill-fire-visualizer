#ifndef OpenGLContext_h
#define OpenGLContext_h

#include <GLFW/glfw3.h>

class OpenGLContext
{
public:
    int screenHeight;
    int screenWidth;
    GLFWwindow* window;
    int initOpenGL(GLint width, GLint height, const char* title);
    unsigned int compileShaderProgram(const char* vertexPath, const char* fragmentPath);
    
private:
};

#endif
