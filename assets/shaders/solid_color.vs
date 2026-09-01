#version 330 core

in vec3 a_Position;
in vec4 a_Color;

uniform vec4 u_Color;
out vec4 out_color;

void main()
{
    out_color = u_Color * a_Color;
    gl_Position = vec4(a_Position.x, a_Position.y, a_Position.z, 1.0);
}