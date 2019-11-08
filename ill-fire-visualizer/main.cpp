#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sndfile.hh>
#include <fftw3.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

#define GLEW_STATIC
#define BUFFER_LEN 2048
#define REAL 0
#define IMAGINARY 1

const GLint WIDTH = 800, HEIGHT = 600;

using namespace std;

int main(int argc, const char *argv[])
{
    /* handling of input file */
    if (argc != 2)
    {
        cout << "Usage: ill-fire-visualizer input_file" << endl;
    }
    
    const char *input_file = argv[1];
    SNDFILE *track_info;
    SF_INFO sf_info;
    memset(&sf_info, 0, sizeof(SF_INFO));

    if (! (track_info = sf_open(input_file, SFM_READ, &sf_info)))
    {
        cout << "Unable to open input file" << endl;
        return -1;
    }
    
    int time_of_one_buffer = BUFFER_LEN / ((float) sf_info.samplerate) * 1000;
    cout << glfwGetVersionString() << endl;
    
    fftw_complex *current_frequencies;
    fftw_plan p;
    
    current_frequencies = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BUFFER_LEN);
    p = fftw_plan_dft_1d(BUFFER_LEN, current_frequencies, current_frequencies, FFTW_FORWARD, FFTW_ESTIMATE);
    
    float current_frames [BUFFER_LEN];
    sf_count_t frames_read;
    
    /* window initialization */
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
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "ILL FIRE", nullptr, nullptr);
    int screenWidth, screenHeight;
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
    
    /* shaders */
    Shader myShaders("vertex.sh", "fragment.sh");
    
    /* vertex data */
    float vertices[] = {
        -0.5f, -0.5f, // bottom-left
        0.5f, -0.5f, // bottom-right
        0.5f,  0.5f, // top-right
        -0.5f, 0.5f // top-left
    };
    
    /* index data */
    unsigned int indices[] = {
        0, 1, 3, // triangle 1
        1, 2, 3 // triangle 2
    };
    
    /* vertex buffers and attributes */
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    /* bind VAO */
    glBindVertexArray(VAO);
    
    /* bind and set VBO */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    /* bind and set EBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    /* configure vertex attributes */
    GLint positionLocation = glGetAttribLocation(myShaders.ID, "vPosition");
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(positionLocation);
    
    /* main rendering loop */
    while (!glfwWindowShouldClose(window))
    {
        /* reading and transforming frames */
//        if ((frames_read = sf_read_float(track_info, current_frames, BUFFER_LEN)))
//        {
//            for (int i = 0; i < BUFFER_LEN; ++i)
//            {
//                current_frequencies[i][REAL] = current_frames[i];
//                current_frequencies[i][IMAGINARY] = 0;
//            }
//
//            fftw_execute(p);
//        }
        
        glClearColor(0.5f, 0.5f, 0.5f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        myShaders.use();
        
        /* send uniform color to shader */
        GLint colorLocation = glGetUniformLocation(myShaders.ID, "vColor");
        float color[] = {
            0.0f, 0.0f, 1.0f, 0.0f
        };
        glUniform4fv(colorLocation, 1, color);
        
        /* send projection to shader */
        glm::mat4 projectionMatrix = glm::ortho(-400.0f, 400.0f, -300.0f, 300.0f);
        GLint projectionLocation = glGetUniformLocation(myShaders.ID, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
        
        /* send modelView to shader */
        glm::mat4 modelView = glm::mat4(1.0f);
        modelView = glm::scale(modelView, glm::vec3(20.0f, 400.0f, 0.0f));
        GLint modelViewLocation = glGetUniformLocation(myShaders.ID, "modelView");
        glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, &modelView[0][0]);
    
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    /* clean-up */
    
    fftw_destroy_plan(p);
    fftw_free(current_frequencies);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    
    if (sf_close(track_info) != 0)
    {
        cout << "Unable to close input file" << endl;
        return -1;
    }
    
    return 0;
}
