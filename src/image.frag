#version 330 core
uniform sampler2DRect tex_src;
varying vec4 ourColor;
in vec2 Pos;
out vec4 FragColor;

void main()
{
    FragColor = texture2DRect(tex_src, Pos) + ourColor;
}
