#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
const GLint WIDTH = 800, HEIGHT = 800;

#include <sndfile.hh>

int main(int argc, const char* argv[])
{
    /* handling of input file */
    if (argc != 2)
    {
        std::cout << "Usage: ill-fire-visualizer input_file" << std::endl;
    }
    
    const char* input_file = argv[1];
    SNDFILE *track_info;
    SF_INFO sf_info;
    memset(&sf_info, 0, sizeof(SF_INFO));

    if (! (track_info = sf_open(input_file, SFM_READ, &sf_info)))
    {
        std::cout << "Unable to open input file" << std::endl;
        
        return -1;
    }
    
    std::cout << "channels: " << sf_info.channels << std::endl;
    std::cout << "format: " << sf_info.format << std::endl;
    std::cout << "frames: " << sf_info.frames << std::endl;
    std::cout << "sample rate: " << sf_info.samplerate << std::endl;
    std::cout << "sections: " << sf_info.sections << std::endl;
    
    /* window initialization */
    glfwInit();
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "ILL FIRE", nullptr, nullptr);
    
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        
        glfwTerminate();
        
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    glewExperimental = GL_TRUE;
    
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        
        return -1;
    }
    
    glViewport(0, 0, screenWidth, screenHeight);
    
    /* main rendering loop */
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();

    /* clean-up */
    if (sf_close(track_info) != 0)
    {
        std::cout << "Unable to close input file" << std::endl;
    }
    
    return 0;
}
