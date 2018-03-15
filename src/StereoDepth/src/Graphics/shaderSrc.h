
#pragma once

const char* vertShaderYUYV420 = "precision highp float;\n"
		"attribute vec3 a_position;\n"
		"attribute vec2 a_texcoord;\n"
		"varying vec2 v_texcoord;\n"
		"varying vec4 v_shifttexcoord;\n"
		"uniform float u_width;"

		"void main()\n"
		"{\n"
		"   gl_Position = vec4(a_position, 1.0);\n"
		"   v_texcoord= a_texcoord;\n"

		"   vec4 shifttexcoord = vec4(a_texcoord.x, a_texcoord.x - 0.25, a_texcoord.x - 0.5, a_texcoord.x - 0.75);\n"
		"	v_shifttexcoord = shifttexcoord * u_width;"
		"}";

const char* fragShaderYUYV420 = "precision highp float;\n"
		"varying vec2 v_texcoord;\n"
		"varying vec4 v_shifttexcoord;\n"
		"uniform sampler2D texture0;\n"

		"void main()\n"
		"{\n"
		"	vec4 yuyv = texture2D(texture0, v_texcoord);\n"

		"	float r, g, b, y, u, v;\n"
		"	u = yuyv.y; v = yuyv.w;\n"

		"	float coordx;\n"
		"	if (v_texcoord.x < 0.25) "
		"		coordx = v_shifttexcoord.x;\n"
		"	else if (v_texcoord.x < 0.5) "
		"		coordx = v_shifttexcoord.y;\n"
		"	else if (v_texcoord.x < 0.75) "
		"		coordx = v_shifttexcoord.z;\n"
		"	else"
		"		coordx = v_shifttexcoord.w;\n"

		"	coordx = fract(coordx);\n"
		"	y = (1.0-coordx)*yuyv.x + coordx * yuyv.z;\n"

		"	y = 1.1643*(y-0.0625);\n"
		"	u = u - 0.5;\n"
		"	v = v - 0.5;\n"
		"	r = y + 1.5958 * v;\n"
		"	g = y - 0.39173 * u - 0.81290 * v;\n"
		"	b = y + 2.017 * u;\n"

		"	gl_FragColor=vec4(r,g,b,1.0);\n"
		"}";

const char* fragShaderYUYV_Gray = "precision highp float;\n"
		"varying vec2 v_texcoord;\n"
		"varying vec4 v_shifttexcoord;\n"
		"uniform sampler2D texture0;\n"

		"void main()\n"
		"{\n"
		"	vec4 yuyv = texture2D(texture0, v_texcoord);\n"
		"	float y;\n"

		"	float coordx;\n"
		"	if (v_texcoord.x < 0.25) "
		"		coordx = v_shifttexcoord.x;\n"
		"	else if (v_texcoord.x < 0.5) "
		"		coordx = v_shifttexcoord.y;\n"
		"	else if (v_texcoord.x < 0.75) "
		"		coordx = v_shifttexcoord.z;\n"
		"	else"
		"		coordx = v_shifttexcoord.w;\n"

		"	coordx = fract(coordx);\n"
		"	y = (1.0-coordx)*yuyv.x + coordx * yuyv.z;\n"

		"	gl_FragColor=vec4(y,y,y,1.0);\n"
		"}";

const char* fragShaderGray16 = "precision mediump float;\n"
		"varying vec2 v_texcoord;\n"
		"uniform sampler2D texture0;\n"
		"uniform float u_coef;\n"
		"void main()\n"
		"{\n"
		"	vec4 col = texture2D(texture0, v_texcoord);\n"

		// cas où rgb sont mappé sur 16 bits
		//"	float y = col.r + col.g *.03125;\n"		// r + (g * 2^(-5))

		// cas où rgb sont mappé sur 8 bits
		"	float y = u_coef * (col.g * 8.0 + col.b * 0.125);\n"	// 1.0 * (g * 2^3) + (b * 2^(-3))
		"	if(col.r == 1.0)\n"
		"		y = 0.0;\n"
		//"	float y = u_coef * (col.r + (col.g * .03125 + col.b * 0.0004883));\n"

		"	gl_FragColor = vec4(y,y,y,1.0);\n"
		"}";
