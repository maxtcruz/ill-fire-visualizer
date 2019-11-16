#ifndef OpenGLContext_h
#define OpenGLContext_h

class OpenGLContext
{
public:
    int screenHeight;
    int screenWidth;
    GLFWwindow* window;
    int initOpenGL(GLint width, GLint height, const char* title);
    unsigned int compileShaderProgram();
    
private:
};

#endif
