/*
 * RenderTexture.h
 *
 *  Created on: Feb 4, 2018
 *      Author: Thibault
 */
#pragma once
#include <iostream>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <glm/glm.hpp>	// gl math
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "esUtil/esUtil.h"
#include "Shader.h"
#include "Camera.h"
#include "../tools/GPIO.h"

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


class Graphics3D {
	//----------------------------------------------//
	//					Public						//
	//----------------------------------------------//
public:
	//--------------------------------------//
	//				Constructors			//
	//--------------------------------------//
	Graphics3D(const int win_width = 1920, const int win_height = 1200);
	virtual ~Graphics3D();

	//--------------------------------------//
	//				Methods					//
	//--------------------------------------//
	void drawPoints(cv::Mat& dispSrc, cv::Mat& imSrc, float coef = 1.0);

	//----------------------------------------------//
	//					Private						//
	//----------------------------------------------//
private:
	void Init();
	void generatePointCloud(cv::Mat& disp, float* const vertices, const float maxZ, const float coef);

	//--------------------------------------//
	//				Attributes				//
	//--------------------------------------//
	ESContext esContext;
	//Shader shader;	// not working ?
	Camera camera;

	GLuint programID;

	// OpenGL objects indices
	GLuint VBO;			// Vertex Buffer Object
	GLuint EBO;			// Element Buffer Object
	GLuint texture[NB_TEXTURE];
	// Attributes locations:
	GLint a_position;
	GLint a_texcoord;
	// Uniforms locations:
	GLint texSelect;
	GLint u_mvp;

	const int NB_ATTRIB = 5;
	GLfloat* vertices;
	GLuint* indices;
	int NB_INDICES;
	size_t sizeByteVertices;
	bool firstFrame = true;

	GPIO btns;

	const char* vertShaderSrc = "precision mediump float;\n"
			"attribute vec3 a_position;\n"
			"attribute vec2 a_texcoord;\n"
			"varying vec2 v_texcoord;\n"
			"uniform mat4 u_mvp;\n"
			"\n"
			"void main()\n"
			"{\n"
			"    gl_Position = u_mvp * vec4(a_position, 1.0);\n"
			"    gl_PointSize = 10.0 / gl_Position.z;\n"
			"    v_texcoord= a_texcoord;\n"
			"}";
	const char* fragShaderSrc = "precision mediump float;\n"
			"varying vec2 v_texcoord;\n"
			"uniform sampler2D texture0;\n"
			"void main()\n"
			"{\n"
			"	gl_FragColor = texture2D(texture0, v_texcoord);\n"
			"}";
};


