#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GLFW/glfw3.h>
#include "ShaderProgram.h"

using namespace std;

ShaderProgram::ShaderProgram()
{
    const char* vShaderCode = getVertexShader();
    const char* fShaderCode = getFragmentShader();
    
    unsigned int vertex, fragment;
    
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}
    
void ShaderProgram::checkCompileErrors(unsigned int shader, string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
        }
    }
}

const char* ShaderProgram::getVertexShader()
{
    return "#version 330 core\n"
    "in vec4 vPosition;\n"
    
    "uniform mat4 projection;\n"
    "uniform mat4 modelView;\n"
    "uniform vec4 vColor;\n"
    
    "out vec4 outColor;\n"
    
    "void main()\n"
    "{\n"
        "outColor = vColor;\n"
        "gl_Position = projection * modelView * vPosition;\n"
    "}";
}

const char* ShaderProgram::getFragmentShader()
{
    return "#version 330 core\n"
    "in vec4 outColor;\n"
    
    "out vec4 fragColor;\n"
    
    "void main()\n"
    "{\n"
        "fragColor = outColor;\n"
    "}";
}


