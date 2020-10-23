#version 330 core
uniform sampler2DRect texture0;
in vec2 Pos;
out vec4 FragColor;

void main()
{

    FragColor = texture2DRect(texture0, Pos);
}
