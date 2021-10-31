#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec3 texArrayCoords;

uniform sampler2DArray arrayTexture;

void main()
{
    FragColor = texture(arrayTexture, texArrayCoords);
}