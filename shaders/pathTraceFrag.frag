#version 400

uniform sampler2D pathTraceTex;
in vec2 VTexCoord;

out vec4 FragColor;

void main(void)
{
    vec4 colour = texture(pathTraceTex, VTexCoord);
    FragColor = colour;
}
