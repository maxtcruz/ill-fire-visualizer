#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
const GLint WIDTH = 800, HEIGHT = 600;

#include <sndfile.hh>
#define BUFFER_LEN 2048

#include <fftw3.h>
#define REAL 0
#define IMAGINARY 1

int main(int argc, const char *argv[])
{
    /* handling of input file */
    if (argc != 2)
    {
        std::cout << "Usage: ill-fire-visualizer input_file" << std::endl;
    }
    
    const char *input_file = argv[1];
    SNDFILE *track_info;
    SF_INFO sf_info;
    memset(&sf_info, 0, sizeof(SF_INFO));

    if (! (track_info = sf_open(input_file, SFM_READ, &sf_info)))
    {
        std::cout << "Unable to open input file" << std::endl;
        return -1;
    }
    
    fftw_complex *current_frequencies;
    fftw_plan p;
    
    current_frequencies = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BUFFER_LEN);
    p = fftw_plan_dft_1d(BUFFER_LEN, current_frequencies, current_frequencies, FFTW_FORWARD, FFTW_ESTIMATE);
    
    float current_frames [BUFFER_LEN];
    sf_count_t frames_read;
    
    /* window initialization */
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
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
    while (!glfwWindowShouldClose(window))
    {
        /* reading and transforming frames */
        if ((frames_read = sf_read_float(track_info, current_frames, BUFFER_LEN)))
        {
            for (int i = 0; i < BUFFER_LEN; ++i)
            {
                current_frequencies[i][REAL] = current_frames[i];
                current_frequencies[i][IMAGINARY] = 0;
            }
            
            fftw_execute(p);
        }
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    /* clean-up */
    
    fftw_destroy_plan(p);
    fftw_free(current_frequencies);
    glfwTerminate();
    
    if (sf_close(track_info) != 0)
    {
        std::cout << "Unable to close input file" << std::endl;
    }
    
    return 0;
}
