#version 330 core
uniform sampler2DRect tex_src;
varying vec4 ourColor;
in vec2 Pos;
out vec4 FragColor;

void main()
{
    FragColor = texture2DRect(tex_src, Pos) + ourColor;//
}
//uniform sampler2DRect tex_src;
//uniform sampler2DRect tex_tar;
//varying vec4 membrane;
//void main()
//{
//    vec4 source = texture2DRect(tex_src,gl_TexCoord[1].st);
//    vec4 target = texture2DRect(tex_tar,gl_TexCoord[0].st);
//    gl_FragColor =source + membrane;
//}
