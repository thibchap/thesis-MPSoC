#pragma once

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader
{
public:
	// The program ID
	GLuint ID;
	Shader() {}
	// Constructor reads and builds the shader
	Shader(const char* vertexSrcPath, const char* fragmentSrcPath)
	{
		// 1. retrieve the vertex/fragment source code from filePath
		// ---------------------------------------------------------
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			// open files
			vShaderFile.open(vertexSrcPath);
			fShaderFile.open(fragmentSrcPath);
			std::stringstream vShaderStream;
			std::stringstream fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// convert stream to string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();

		// 2. compile shaders
		// ------------------
		GLuint vertex;
		GLuint fragment;
		GLint success;
		GLchar infoLog[512];

		// vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		};

		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		};

		// shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		// print linking errors if any
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}


	void use()
	{
		glUseProgram(ID);
	}


	void setUniform(const std::string &name, GLboolean value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (GLint)value);
	}

	void setUniform(const std::string &name, GLint value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setUniform(const std::string &name, GLfloat value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setUniform(const std::string &name, GLfloat x, GLfloat y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}

	void setUniform(const std::string &name, GLfloat x, GLfloat y, GLfloat z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}

	void setUniform(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
};
