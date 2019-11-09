#include <iostream>
#include <thread>
#include <stack>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sndfile.hh>
#include <complex.h>
#include <fftw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

#define GLEW_STATIC
#define FFT_SIZE 256
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
    
    /* fft */
    const char *inputFile = argv[1];
    SNDFILE *trackInfo;
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(SF_INFO));

    if (! (trackInfo = sf_open(inputFile, SFM_READ, &sfInfo)))
    {
        cout << "Unable to open input file" << endl;
        return -1;
    }
    
    int timeOfOneBufferMicroseconds = FFT_SIZE / ((float) sfInfo.samplerate) * 1000000;
    int numChannels = sfInfo.channels;
    
    fftw_complex *inAmplitudes = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fftw_complex *outFrequencies = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fftw_plan plan = fftw_plan_dft_1d(FFT_SIZE , inAmplitudes, outFrequencies, FFTW_FORWARD, FFTW_ESTIMATE);
    
    float currentFrames [FFT_SIZE * numChannels];
    sf_count_t framesRead;
    
    /* opengl */
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
    
    /* compile shaders */
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
        auto timeOfNextIteration = chrono::steady_clock::now() + chrono::microseconds(timeOfOneBufferMicroseconds);
        
        /* reading and transforming frames */
        if ((framesRead = sf_read_float(trackInfo, currentFrames, FFT_SIZE * numChannels)))
        {
            for (int i = 0; i < FFT_SIZE * numChannels; i += numChannels)
            {
                // average left channel amplitude values
                float sumAmplitude = 0.0f;
                for (int j = 0; j < numChannels; ++j)
                {
                    sumAmplitude += currentFrames[i + j];
                }
                inAmplitudes[i / numChannels][REAL] = sumAmplitude / numChannels;
                inAmplitudes[i / numChannels][IMAGINARY] = 0;
            }

            fftw_execute(plan);
        }
    
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
        
        /* send modelView to shader & draw */
        stack<glm::mat4> modelView;
        modelView.push(glm::mat4(1.0f));
        
        for (int i = 0; i < FFT_SIZE / 2; ++i) {
            float frequencyBinThickness = ((float) screenWidth) / FFT_SIZE;
            float frequencyBinMagnitude = sqrt(pow(outFrequencies[i][REAL], 2) + pow(outFrequencies[i][IMAGINARY], 2));
            float firstFrequencyBinXCoord = -1 * screenWidth * 0.5f + frequencyBinThickness;
            float frequencyBinDeltaXCoord = 2 * frequencyBinThickness;
            
            modelView.push(modelView.top());
            modelView.top() = glm::translate(modelView.top(), glm::vec3(firstFrequencyBinXCoord + (i * frequencyBinDeltaXCoord), -300.0f, 0.0f));
            modelView.top() = glm::scale(modelView.top(), glm::vec3(frequencyBinThickness, 50.0f + (100 * frequencyBinMagnitude), 1.0f));
            modelView.top() = glm::translate(modelView.top(), glm::vec3(0.0f, 0.5f, 0.0f));
            
            GLint modelViewLocation = glGetUniformLocation(myShaders.ID, "modelView");
            glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, &modelView.top()[0][0]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            
            modelView.pop();
        }
        
        modelView.pop();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
        
        this_thread::sleep_until(timeOfNextIteration);
    }

    /* clean-up */
    
    fftw_destroy_plan(plan);
    fftw_free(outFrequencies);
    fftw_free(inAmplitudes);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    
    if (sf_close(trackInfo) != 0)
    {
        cout << "Unable to close input file" << endl;
        return -1;
    }
    
    return 0;
}
