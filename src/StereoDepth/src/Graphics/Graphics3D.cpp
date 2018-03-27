/*
 * RenderTexture.cpp
 *
 *  Created on: Feb 4, 2018
 *      Author: Thibault
 */

#include "Graphics3D.h"

#define TEX_FORMAT 	GL_RGB
#define TEX_TYPE 	GL_UNSIGNED_SHORT_5_6_5
#define TEX_CV_CVT 	cv::COLOR_BGR2BGR565

Graphics3D::Graphics3D(const int width, const int height)
: camera(glm::vec3(2.8f, -1.5f, 3.5f)), vertices(nullptr), indices(nullptr),
  NB_INDICES(0), btns(PUSH_BTN_ADDR, true)
//shader("shader/videoTest.vs", "shader/videoTest_1.fs")
{
	// 1. Initialize the OpenGL ES context
	esInitContext(&esContext);
	//esContext.userData = &userData;	// bind userData to esContext

	// 2. Create a window
	esCreateWindow(&esContext, "OpenGL ES Window", width, height, ES_WINDOW_RGB);

	// 3. Initialize the context and userData
	Init();

	// optional : disable V-Sync
	esSwapInterval(&esContext, 0);
}

Graphics3D::~Graphics3D() {
	delete[] vertices;
	delete[] indices;
}

void Graphics3D::Init() {
	programID = esLoadProgram(vertShaderSrc, fragShaderSrc);

	// Get location of the attributes after the shader is linked
	a_position = glGetAttribLocation(programID, "a_position");
//	a_texcoord = glGetAttribLocation(programID, "a_texcoord");
	if(NB_TEXTURE > 1)
		texSelect = glGetUniformLocation(programID, "u_texSelect");
	u_mvp = glGetUniformLocation(programID, "u_mvp");

	// set-up vertex data, buffers and attributes
	// ------------------------------------------
	// Generate, bind and set data of the Vertex Buffer Object
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	const GLuint POS_SIZE = 3;		// position is 3 dimensions
//	const GLuint TEXCOORD_SIZE = 2;	// texcoord is 2 dimensions
//	const GLuint STRIDE = (POS_SIZE + TEXCOORD_SIZE) * sizeof(GLfloat);	// stride in bytes between each attribute
	const GLuint STRIDE = (POS_SIZE) * sizeof(GLfloat);	// stride in bytes between each attribute
	size_t offset = 0;

	// position attribute
	glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)offset);
	glEnableVertexAttribArray(a_position);
	offset += POS_SIZE * sizeof(float);
	// texture attribute ---> NO LONGER NEEDED (see shader)
//	glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, STRIDE, (void*)offset);
//	glEnableVertexAttribArray(a_texcoord);
//	offset += TEXCOORD_SIZE * sizeof(float);

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
	const int W = 1280;	// todo
	const int H = 720;
	vertices = new GLfloat[W * H * NB_ATTRIB];
	sizeByteVertices = W * H * NB_ATTRIB * sizeof(float);

	NB_INDICES = W * H * 3 * 2;
	indices = new GLuint[NB_INDICES];
	int idx = 0;
	// Init indices array
	for (int i = 0; i < (H - 1); i++) {
		for (int j = 0; j < (W - 1); j++) {
			const uint s = i * W + j;
			indices[idx++] = s;
			indices[idx++] = s + W;
			indices[idx++] = s + 1;
			indices[idx++] = s + W;
			indices[idx++] = s + 1;
			indices[idx++] = s + W + 1;
		}
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NB_INDICES * sizeof(GLuint), indices, GL_STATIC_DRAW);

	camera.Yaw += 20.0f;	// rotate camera
}


void Graphics3D::drawPoints(cv::Mat& disp, cv::Mat& im, float coef) {
	generatePointCloud(disp, vertices, -10000.0f, coef);
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

	const uint val = btns.read();
	//std::cout << "btns " << val << std::endl;
	glm::vec3 move;
	if(val == 1) {
		camera.Yaw += 10.0f;
		camera.updateCameraVectors();
	}
	else if(val == 2) {
		camera.Position += camera.Front* 0.1f;
	}
	else if(val == 4) {
		camera.Yaw -= 10.0f;
		camera.updateCameraVectors();
	}

	glm::mat4 transform;
	float ratio = disp.rows / (float)disp.cols;
	glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(5.0f, 5.0f * ratio, 5.0f));
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)esContext.width / (float)esContext.height, 0.1f, 100.0f);
	transform = projection * view * model;
	glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(transform));

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeByteVertices, vertices, GL_DYNAMIC_DRAW);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);	// gray background
	glClear(GL_COLOR_BUFFER_BIT);			// clear color buffer
	glDrawArrays(GL_POINTS, 0, im.rows * im.cols);
	//glDrawElements(GL_TRIANGLES, NB_INDICES, GL_UNSIGNED_INT, 0);
	//glDrawElements(GL_POINTS, NB_INDICES, GL_UNSIGNED_INT, 0);
	eglSwapBuffers(esContext.eglDisplay, esContext.eglSurface);
}


void Graphics3D::generatePointCloud(cv::Mat& im, GLfloat* vertices, const float maxZ, const float coef) {
	const int H = im.rows;
	const int W = im.cols;
	const float H_f = (float) -H;
	const float W_f = (float) W;
	const int NB_ATTRIB = 3; // 5; ////////////// TEST
	uint s, _s;

	if (vertices == nullptr or indices == nullptr) {
		std::cerr << "[Graphics3D] Error vertices pointer invalid" << std::endl;
		return;
	}

	// Init x and y coordinates, only once
	static bool init = true;
	if (init) {
		init = false;
#if !__SDSCC__
#pragma omp parallel for collapse(2)
#endif
		for (int i = 0; i < H; i++) {
			for (int j = 0; j < W; j++) {
				s = (i * W + j) * NB_ATTRIB;
				vertices[s] = j / W_f;		// pos x
				vertices[s + 1] = i / H_f;		// pos y
			}
		}
	}

	// Set depth values (inverse of disparity)
#if !__SDSCC__
#pragma omp parallel for collapse(2)
#endif
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			_s = (i * W + j);
			s = _s * NB_ATTRIB;
			float disp = ((int16_t*)im.data)[_s];		// disparity
			vertices[s + 2] = maxZ;
			if (disp > 50.0f) {
				vertices[s + 2] = -180.0f / disp;	// pos z
			}
		}
	}
}
