#ifndef ShaderProgram_h
#define ShaderProgram_h

using namespace std;

class ShaderProgram
{
public:
    unsigned int ID;
    ShaderProgram(const char* vertexPath, const char* fragmentPath);
    
private:
    void checkCompileErrors(unsigned int shader, string type);
};

#endif
