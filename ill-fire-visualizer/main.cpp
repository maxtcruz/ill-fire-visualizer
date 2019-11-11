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
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "OpenGLContext.h"

#define GLEW_STATIC
#define FFT_SIZE 512
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
    const char *inputFile = argv[1];
    
    /* fft */
    SNDFILE *trackInfo;
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(SF_INFO));

    if (! (trackInfo = sf_open(inputFile, SFM_READ, &sfInfo)))
    {
        cout << "Unable to open input file" << endl;
        return -1;
    }
    
    int timeOfOneBufferNanoseconds = ((float) FFT_SIZE) / ((float) sfInfo.samplerate) * 1000000000;
    
    fftw_complex *inAmplitudes = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fftw_complex *outFrequencies = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fftw_plan plan = fftw_plan_dft_1d(FFT_SIZE , inAmplitudes, outFrequencies, FFTW_FORWARD, FFTW_ESTIMATE);
    
    float currentFrames [FFT_SIZE * sfInfo.channels];
    sf_count_t framesRead;
    
    /* init opengl */
    OpenGLContext openGLContext;
    if (openGLContext.initOpenGL(WIDTH, HEIGHT, "ill fire") == -1)
    {
        cout << "Failed to initialize OpenGL" << endl;
        return -1;
    }
    
    /* compile shaders */
    unsigned int shaderProgramID = openGLContext.compileShaderProgram("vertex.sh", "fragment.sh");

    float vertices[] = {
        // front of cube
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        // back of cube
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f
    };
    
    unsigned int indices[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
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
    GLint positionLocation = glGetAttribLocation(shaderProgramID, "vPosition");
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(positionLocation);
    
    /* sdl audio */
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        cout << "Unable to initialize SDL audio" << endl;
        return -1;
    }
    
    if (Mix_OpenAudio(sfInfo.samplerate, MIX_DEFAULT_FORMAT, sfInfo.channels, 2048) < 0)
    {
        cout << "Unable to open SDL Mixer audio" << endl;
        return -1;
    }
    
    Mix_Music *sound = Mix_LoadMUS(inputFile);
    if (!sound)
    {
        cout << "Unable to load input file into the SDL Mixer: " << SDL_GetError() << endl;
        return -1;
    }
    
    if (Mix_PlayingMusic() == 0)
    {
        Mix_PlayMusic(sound, 1);
    }
    
    auto timeOfNextLoopIteration = chrono::steady_clock::now();
    
    /* main rendering loop */
    while (!glfwWindowShouldClose(openGLContext.window))
    {
        timeOfNextLoopIteration += chrono::nanoseconds(timeOfOneBufferNanoseconds);
        
        /* reading and transforming frames */
        if ((framesRead = sf_read_float(trackInfo, currentFrames, FFT_SIZE * sfInfo.channels)))
        {
            for (int i = 0; i < FFT_SIZE * sfInfo.channels; i += sfInfo.channels)
            {
                // average all channel amplitude values
                float sumAmplitude = 0.0f;
                for (int j = 0; j < sfInfo.channels; ++j)
                {
                    sumAmplitude += currentFrames[i + j];
                }
                float averageAmplitude = sumAmplitude / sfInfo.channels;
                
                // apply hamming window function to amplitude
                float hammingMultiplier = 0.54f - 0.46f * cos((2 * M_PI * (i / sfInfo.channels)) / (FFT_SIZE - 1));
                float amplitude = averageAmplitude * hammingMultiplier;
                
                inAmplitudes[i / sfInfo.channels][REAL] = amplitude;
                inAmplitudes[i / sfInfo.channels][IMAGINARY] = 0;
            }

            /* transform amplitudes */
            fftw_execute(plan);
        }
        else {
            /* end program if no frames left */
            break;
        }
    
        glClearColor(1.0f, 1.0f, 1.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        
        glUseProgram(shaderProgramID);
        
        /* send projection to shader */
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 10000.0f);
        GLint projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
        
        GLint modelViewLocation = glGetUniformLocation(shaderProgramID, "modelView");
        stack<glm::mat4> modelView;
        modelView.push(glm::lookAt(glm::vec3(-500.0f, 50.0f, 500.0f), glm::vec3(-100.0f, -50.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
        
        GLint colorLocation = glGetUniformLocation(shaderProgramID, "vColor");
        float color[] = {
            0.0f, 0.0f, 0.0f, 0.0f
        };
        
        /* configure and draw each frequency bin */
        for (int i = 0; i < FFT_SIZE / 2; ++i) {
            float frequencyBinThickness = ((float) openGLContext.screenWidth) / FFT_SIZE;
            float frequencyBinMagnitude = sqrt(pow(outFrequencies[i + 1][REAL], 2) + pow(outFrequencies[i + 1][IMAGINARY], 2));
            float firstFrequencyBinXCoord = -1 * openGLContext.screenWidth * 0.5f + frequencyBinThickness;
            float frequencyBinDeltaXCoord = 2 * frequencyBinThickness;
            
            modelView.push(modelView.top());
            modelView.top() = glm::translate(modelView.top(), glm::vec3(firstFrequencyBinXCoord + (i * frequencyBinDeltaXCoord), -300.0f, 0.0f));
            modelView.top() = glm::scale(modelView.top(), glm::vec3(frequencyBinThickness, min(50.0f + (10 * frequencyBinMagnitude), openGLContext.screenHeight - 50.0f), 10.0f));
            modelView.top() = glm::translate(modelView.top(), glm::vec3(0.0f, 0.5f, 0.0f));
            
            /* send modelView to shader */
            glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, &modelView.top()[0][0]);
            
            /* send color to shader */
            color[0] = ((float) (FFT_SIZE / 2 - i)) / (FFT_SIZE / 2);
            color[2] = ((float) i) / (FFT_SIZE / 2);
            glUniform4fv(colorLocation, 1, color);
            
            /* draw frequency bin */
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            
            modelView.pop();
        }
        
        modelView.pop();
        
        glfwPollEvents();
        glfwSwapBuffers(openGLContext.window);
        
        /* halt loop execution to stay in sync with audio */
        this_thread::sleep_until(timeOfNextLoopIteration);
    }

    /* clean-up */
    Mix_FreeMusic(sound);
    Mix_CloseAudio();
    SDL_CloseAudio();
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    
    fftw_destroy_plan(plan);
    fftw_free(outFrequencies);
    fftw_free(inAmplitudes);

    if (sf_close(trackInfo) != 0)
    {
        cout << "Unable to close input file" << endl;
        return -1;
    }
    
    return 0;
}
