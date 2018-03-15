precision mediump float;
varying vec2 v_texcoord;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform bool u_texSelect;

void main()
{
	if (u_texSelect) {
		gl_FragColor = texture2D(texture1, v_texcoord);
	}
	else {
		gl_FragColor = texture2D(texture0, v_texcoord);
	}
}