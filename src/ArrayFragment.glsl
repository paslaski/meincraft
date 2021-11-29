#version 330 core

out vec4 FragColor;

in float sunlightLevel;
in vec3 texArrayCoords;

uniform sampler2DArray arrayTexture;

void main()
{
    FragColor = vec4(texture(arrayTexture, texArrayCoords).xyz * sunlightLevel, 1.0f);
}