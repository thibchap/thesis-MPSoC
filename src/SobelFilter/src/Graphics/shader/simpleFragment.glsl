precision mediump float; // indicates the FP and integer precision

uniform sampler2D texture0;

varying vec2 v_texcoord; // input textures coordinates from vertex shader

void main (void)
{
gl_FragColor = texture2D(texture0, v_texcoord);
}