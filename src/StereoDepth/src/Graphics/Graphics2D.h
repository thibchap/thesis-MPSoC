/*
 * RenderTexture.h
 *
 *  Created on: Feb 2, 2018
 *      Author: Thibault
 */
#pragma once
#include <iostream>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include "esUtil/esUtil.h"
#include "Shader.h"
#if __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#define __ARM_NEON__
#define __ARM_NEON
#else
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#endif

#define NB_TEXTURE 1
#define G_WINDOW_FACTOR 0.9f	// 1080 / 1200 = 0.9

class Graphics2D {
	//----------------------------------------------//
	//					Public						//
	//----------------------------------------------//
public:
	//--------------------------------------//
	//				Constructors			//
	//--------------------------------------//
	Graphics2D(const int win_width = 1920, const int win_height = 1200);
	virtual ~Graphics2D();

	//--------------------------------------//
	//				Methods					//
	//--------------------------------------//
	void drawImageBGR565(cv::Mat& imSrc);	//< Warning! Expect image BGR565
	void drawImageYUYV(cv::Mat& imSrc);
	void drawImageGray8(cv::Mat& imSrc);
	void drawImageGray16(cv::Mat& imSrc, GLfloat coef = 1.0);

	void drawSobel(cv::Mat& imSrc);	//< Warning! Expect image BGR565

	//----------------------------------------------//
	//					Private						//
	//----------------------------------------------//
private:
	void Init();
	//--------------------------------------//
	//				Attributes				//
	//--------------------------------------//
	ESContext esContext;
	//Shader shader;	// not working ?

	GLuint programID;
	GLuint programYUYV;
	GLuint programGray16;
	GLuint programSobel;

	// OpenGL objects indices
	GLuint VBO;			// Vertex Buffer Object
	GLuint texture[NB_TEXTURE];
	// Attributes locations:
	GLint a_position;
	GLint a_texcoord;
	// Uniforms locations:
	GLint texSelect;
	GLint u_width;
	GLint u_coef;

	const GLfloat vertices[30] = {
			// positions:(x,y,z)    // texture coords:(s,t)
			-1.0f, -1.0f * G_WINDOW_FACTOR, 0.0f,		0.0f, 1.0f,   // bottom left
			1.0f, -1.0f * G_WINDOW_FACTOR, 0.0f,		1.0f, 1.0f,   // bottom right
			1.0f,  1.0f * G_WINDOW_FACTOR, 0.0f,		1.0f, 0.0f,   // top right
			-1.0f, -1.0f * G_WINDOW_FACTOR, 0.0f,		0.0f, 1.0f,   // bottom left
			1.0f,  1.0f * G_WINDOW_FACTOR, 0.0f,		1.0f, 0.0f,   // top right
			-1.0f,  1.0f * G_WINDOW_FACTOR, 0.0f,		0.0f, 0.0f    // top left
	};

	const char* vertShaderSrc = "precision mediump float;\n"
			"attribute vec3 a_position;\n"
			"attribute vec2 a_texcoord;\n"
			"varying vec2 v_texcoord;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = vec4(a_position, 1.0);\n"
			"    v_texcoord= a_texcoord;\n"
			"}";
	const char* fragShaderSrc = "precision mediump float;\n"
			"varying vec2 v_texcoord;\n"
			"uniform sampler2D texture0;\n"
			"void main()\n"
			"{\n"
			"	gl_FragColor = texture2D(texture0, v_texcoord);\n"
			"}";

	const char* fragShaderSobel = "precision mediump float;\n"
			"varying vec2 v_texcoord;\n"
			"uniform sampler2D texture0;\n"
			"uniform vec2 imageSize;\n"
			"\n"
			"void main()\n"
			"{\n"
			"	float dx = 1.0 / imageSize.x;\n"
			"	float dy = 1.0 / imageSize.y;\n"
			"	float gx = 0.0;\n"
			"	float gy = 0.0;\n"
			"	\n"
			"	// Gradient x\n"
			"	gx += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y - dy)).x;\n"
			"	gx += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y     )).x * 2.0;\n"
			"	gx += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y + dy)).x;\n"
			"	gx -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y - dy)).x;\n"
			"	gx -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y     )).x * 2.0;\n"
			"	gx -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y + dy)).x;\n"
			"	// Gradient y\n"
			"	gy += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y - dy)).x;\n"
			"	gy += texture2D(texture0, vec2(v_texcoord.x     , v_texcoord.y - dy)).x * 2.0;\n"
			"	gy += texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y - dy)).x;\n"
			"	gy -= texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y + dy)).x;\n"
			"	gy -= texture2D(texture0, vec2(v_texcoord.x     , v_texcoord.y + dy)).x * 2.0;\n"
			"	gy -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y + dy)).x;\n"
			"	gx = abs(gx) + abs(gy);\n"
			"	gl_FragColor = vec4(gx, gx, gx, 1.0);	// vec1\n"
			"}";

	const char* fragShaderSobelVec4 = "precision mediump float;\n"
			"varying vec2 v_texcoord;\n"
			"uniform sampler2D texture0;\n"
			"uniform vec2 imageSize;\n"
			"const vec4 factor = vec4(0.299, 0.587, 0.114, 0);	// RGB factor for grayscale conversion\n"
			"\n"
			"void main()\n"
			"{\n"
			"	float dx = 1.0 / imageSize.x;\n"
			"	float dy = 1.0 / imageSize.y;\n"
			"	vec4 gx = vec4(0, 0, 0, 0);\n"
			"	vec4 gy = vec4(0, 0, 0, 0);\n"
			"	\n"
			"	// Gradient x\n"
			"	gx += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y - dy));\n"
			"	gx += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y     )) * 2.0;\n"
			"	gx += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y + dy));\n"
			"	gx -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y - dy));\n"
			"	gx -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y     )) * 2.0;\n"
			"	gx -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y + dy));\n"
			"	// Gradient y\n"
			"	gy += texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y - dy));\n"
			"	gy += texture2D(texture0, vec2(v_texcoord.x     , v_texcoord.y - dy)) * 2.0;\n"
			"	gy += texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y - dy));\n"
			"	gy -= texture2D(texture0, vec2(v_texcoord.x - dx, v_texcoord.y + dy));\n"
			"	gy -= texture2D(texture0, vec2(v_texcoord.x     , v_texcoord.y + dy)) * 2.0;\n"
			"	gy -= texture2D(texture0, vec2(v_texcoord.x + dx, v_texcoord.y + dy));\n"
			"	gx = abs(gx) + abs(gy);\n"
			"	//float gray = dot(gx, factor);			// vec4\n"
			"	//gl_FragColor = vec4(gray, gray, gray, 1.0);// gray\n"
			"	gl_FragColor = vec4(gx);					// color\n"
			"}";
};


