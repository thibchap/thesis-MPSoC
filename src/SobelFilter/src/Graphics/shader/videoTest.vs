precision mediump float;
attribute vec3 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;

void main()
{
    gl_Position = vec4(a_position, 1.0);
    v_texcoord= a_texcoord;
}