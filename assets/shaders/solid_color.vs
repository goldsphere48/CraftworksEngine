#version 330 core

in vec3 a_Position;
in vec4 a_Color;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec4 u_Color;
out vec4 out_color;

void main()
{
    out_color = u_Color * a_Color;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}