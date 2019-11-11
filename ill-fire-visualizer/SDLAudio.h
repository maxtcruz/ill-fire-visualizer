#ifndef SDLAudio_h
#define SDLAudio_h

class SDLAudio
{
public:
    int initSDLAudio(const char* inputFile, int sampleRate, int numChannels);
    void play();
    void free();
    
private:
    Mix_Music *sound;
};

#endif
