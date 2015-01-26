#version 400
layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 vertexTexCoord;

out vec2 VTexCoord;
uniform mat4 MVP;

void main(void)
{
    VTexCoord = vertexTexCoord;
    gl_Position = MVP * vec4(vertexPosition,0.0,1.0);
}
