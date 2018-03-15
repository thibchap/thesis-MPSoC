/*
 * RenderTexture.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: Thibault
 */

#include "Graphics2D.h"

#define TEX_FORMAT 	GL_RGB
#define TEX_TYPE 	GL_UNSIGNED_SHORT_5_6_5
#define TEX_CV_CVT 	cv::COLOR_BGR2BGR565

Graphics2D::Graphics2D(const int width, const int height)
//:shader("shader/videoTest.vs", "shader/videoTest_1.fs")
{
	// 1. Initialize the OpenGL ES context
	esInitContext(&esContext);
	//esContext.userData = &userData;	// bind userData to esContext

	// 2. Create a window
	esCreateWindow(&esContext, "OpenGL ES Window", 1920, 1200, ES_WINDOW_RGB);

	// 3. Initialize the context and userData
	Init();

	// optional : disable V-Sync
	esSwapInterval(&esContext, 0);

	// 4. register the draw callback
	//esRegisterDrawFunc(&esContext, Draw);
	// 5. Start main loop
	//esMainLoop(&esContext);
}

Graphics2D::~Graphics2D() {

}

void Graphics2D::Init() {
	programID = esLoadProgram(vertShaderSrc, fragShaderSrc);
	//programSobel = esLoadProgram(vertShaderSrc, fragShaderSobel);
	programSobel = esLoadProgram(vertShaderSrc, fragShaderSobelVec4);

	// Get location of the attributes after the shader is linked
	a_position = glGetAttribLocation(programID, "a_position");
	a_texcoord = glGetAttribLocation(programID, "a_texcoord");
	if(NB_TEXTURE > 1)
		texSelect = glGetUniformLocation(programID, "u_texSelect");

	// set-up vertex data, buffers and attributes
	// ------------------------------------------
	// Generate, bind and set data of the Vertex Buffer Object
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	const GLuint POS_SIZE = 3;		// position is 3 dimensions
	const GLuint TEXCOORD_SIZE = 2;	// texcoord is 2 dimensions
	const GLuint STRIDE = (POS_SIZE + TEXCOORD_SIZE) * sizeof(GLfloat);	// stride in bytes between each attribute
	size_t offset = 0;

	// position attribute
	glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)offset);
	glEnableVertexAttribArray(a_position);
	offset += POS_SIZE * sizeof(float);
	// texture attribute
	glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, STRIDE, (void*)offset);
	glEnableVertexAttribArray(a_texcoord);
	offset += TEXCOORD_SIZE * sizeof(float);

	// texture configuration
	for(int i = 0; i < NB_TEXTURE; i++) {
		glGenTextures(1, &texture[i]);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		// texture wrapping (repeat) and filtering (linear)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	//shader.use();
	//shader.setUniform("u_texSelect", true);
	//shader.setUniform("texture0", 0);
	//shader.setUniform("texture1", 1);
}


void Graphics2D::drawImage(cv::Mat& im) {
	// Set the viewport
	glViewport(0, 0, esContext.width, esContext.height);
	// Use the program object
	//shader.use();
	glUseProgram(programID);

	for(int i = 0; i < NB_TEXTURE; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, TEX_FORMAT, im.cols, im.rows, 0, TEX_FORMAT, TEX_TYPE, im.data);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);	// grey background
	glClear(GL_COLOR_BUFFER_BIT);			// clear color buffer
	glDrawArrays(GL_TRIANGLES, 0, 6);
	eglSwapBuffers(esContext.eglDisplay, esContext.eglSurface);
}

void Graphics2D::drawSobel(cv::Mat& im) {
	// Set the viewport
	glViewport(0, 0, esContext.width, esContext.height);
	// Use the program object
	//shader.use();
	glUseProgram(programSobel);
	glUniform2f(glGetUniformLocation(programSobel, "imageSize"), im.cols, im.rows);

	for(int i = 0; i < NB_TEXTURE; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, TEX_FORMAT, im.cols, im.rows, 0, TEX_FORMAT, TEX_TYPE, im.data);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);	// grey background
	glClear(GL_COLOR_BUFFER_BIT);			// clear color buffer
	glDrawArrays(GL_TRIANGLES, 0, 6);
	eglSwapBuffers(esContext.eglDisplay, esContext.eglSurface);
}
