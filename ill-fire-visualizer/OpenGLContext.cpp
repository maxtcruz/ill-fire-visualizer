#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "OpenGLContext.h"
#include "ShaderProgram.h"

using namespace std;

int OpenGLContext::initOpenGL(GLint width, GLint height, const char* title)
{
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    
    if (window == nullptr)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    
    glViewport(0, 0, screenWidth, screenHeight);
    
    return 0;
}

unsigned int OpenGLContext::compileShaderProgram(const char* vertexPath, const char* fragmentPath)
{
    ShaderProgram shaderProgram("vertex.sh", "fragment.sh");
    return shaderProgram.ID;
}
