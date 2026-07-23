#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec4 color;

out vec4 out_color;

void main()
{
    out_color = color;
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
}