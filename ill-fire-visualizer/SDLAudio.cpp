#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "SDLAudio.h"

using namespace std;

int SDLAudio::initSDLAudio(const char* inputFile, int sampleRate, int numChannels)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        cout << "Unable to initialize SDL audio" << endl;
        return -1;
    }
    
    if (Mix_OpenAudio(sampleRate, MIX_DEFAULT_FORMAT, numChannels, 2048) < 0)
    {
        cout << "Unable to open SDL Mixer audio" << endl;
        return -1;
    }
    
    sound = Mix_LoadMUS(inputFile);
    if (!sound)
    {
        cout << "Unable to load input file into the SDL Mixer: " << SDL_GetError() << endl;
        return -1;
    }
    
    return 0;
}

void SDLAudio::play()
{
    if (Mix_PlayingMusic() == 0)
    {
        Mix_PlayMusic(sound, 1);
    }
}

void SDLAudio::free()
{
    Mix_FreeMusic(sound);
    Mix_CloseAudio();
    SDL_CloseAudio();
}
