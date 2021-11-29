#version 330 core

layout (location = 0) in vec3 aWorldPos;
layout (location = 1) in vec3 aTexArrayCoords;
layout (location = 2) in int alightLevel;

out vec3 texArrayCoords;
out float sunlightLevel;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aWorldPos, 1.0);
    texArrayCoords = aTexArrayCoords;
    sunlightLevel = float(alightLevel & 0xF)/15.0;
}