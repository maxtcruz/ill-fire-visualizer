#ifndef ShaderProgram_h
#define ShaderProgram_h

using namespace std;

class ShaderProgram
{
public:
    unsigned int ID;
    ShaderProgram();
    
private:
    void checkCompileErrors(unsigned int shader, string type);
    const char* getVertexShader();
    const char* getFragmentShader();
};

#endif
