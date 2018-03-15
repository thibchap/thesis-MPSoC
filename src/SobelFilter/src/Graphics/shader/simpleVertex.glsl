precision mediump float;
// uniforms used by the vertex shader
uniform mat4 mvp_matrix; // model-view-projection matrix

// attributes input to the vertex shader
attribute vec3 a_position;	// position value
attribute vec2 a_texcoord;	// texture coordinates

// varying variables – input to the fragment shader
varying vec2 v_texcoord; 	// output texture coordinates

void main (void)
{
v_texcoord = a_texcoord;
gl_Position = mvp_matrix * vec4(a_position, 1.0);
}