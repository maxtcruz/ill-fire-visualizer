#version 330 core
in vec4 vPosition;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec4 vColor;

out vec4 outColor;

void main()
{
    outColor = vColor;
    gl_Position = projection * modelView * vPosition;
}
