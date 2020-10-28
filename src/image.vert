#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 rPos;
layout (location = 2) in float index;
layout (location = 3) in vec3 mem;

varying vec4 ourColor;
out vec2 Pos;
uniform sampler2DRect tex_tar;
uniform sampler2DRect tex_src;
uniform sampler2DRect tex_weight;
uniform sampler2DRect tex_boundarycoords;
uniform sampler2DRect tex_err;
uniform float fx;
uniform float fy;
uniform int fx1;
uniform int fy1;
uniform int boundarysize;
uniform float source_w;
uniform float source_h;
uniform float target_w;
uniform float target_h;

void main()
{
    gl_Position = vec4(aPos+vec2(fx,fy),0.0,1.0);
    gl_Position = gl_Position * 2 - vec4(1.0,1.0,0.0,1.0);
//    ourColor = vec4(0.0);
//    float x,y;
//    bool flag = false;
//    for(int t = 0; t < boundarysize; t++){
//        x = texture2DRect(tex_boundarycoords,vec2(t,0)).a;
//        y = texture2DRect(tex_boundarycoords,vec2(t,1)).a;
//        vec2 tar_coord = vec2(fx1,fy1) + vec2(x,y);
//        vec2 src_coord = vec2(x,y);
//        vec4 err = vec4(texture2DRect(tex_err,vec2(t,0)).a,texture2DRect(tex_err,vec2(t,1)).a,texture2DRect(tex_err,vec2(t,2)).a,1.0)
//                -  vec4(texture2DRect(tex_err,vec2(t,3)).a,texture2DRect(tex_err,vec2(t,4)).a,texture2DRect(tex_err,vec2(t,5)).a,0.0);
//        vec4 err = texture2DRect(tex_tar, tar_coord) - texture2DRect(tex_src, src_coord);
//        float w = texture2DRect(tex_weight,vec2(t,index)).a;
//        ourColor += err * w ;
//    }
    ourColor = vec4(mem/255.0,1.0);
    Pos = rPos.xy;
}
