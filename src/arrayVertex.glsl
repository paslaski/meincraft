#version 330 core

layout (location = 0) in vec3 aWorldPos;
layout (location = 1) in vec3 aTexArrayCoords;

out vec3 texArrayCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aWorldPos, 1.0);
    texArrayCoords = aTexArrayCoords;
}