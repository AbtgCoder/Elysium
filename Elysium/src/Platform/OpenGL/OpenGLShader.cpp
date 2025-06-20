#include "Platform/OpenGL/OpenGLShader.h"

#include "core/Logger.h"

#include <glad/glad.h>

#include <fstream>
#include <sstream>

OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	: m_Name(name)
{
	OpenGLShader(vertexSrc, fragmentSrc);
}

OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
{
	// 1) retrieve the vertex/fragment src code from the given filepaths
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
		vShaderFile.open(vertexSrc.c_str());
		fShaderFile.open(fragmentSrc.c_str());
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		Logger::Log("couldnt read shader file successfully", "Renderer", LOG_TYPE::CRITICAL);
	}
	
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// 2) compile the shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	CheckCompileErrors(vertex, "VERTEX");
	// fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	CheckCompileErrors(fragment, "FRAGMENT");

	// 3) make shader program and link the shaders
	m_RendererID = glCreateProgram();
	glAttachShader(m_RendererID, vertex);
	glAttachShader(m_RendererID, fragment);
	glLinkProgram(m_RendererID);
	CheckCompileErrors(m_RendererID, "PROGRAM");

	// 4) delete the shaders as they're linked inton our program now and so no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

OpenGLShader::~OpenGLShader()
{
	glDeleteProgram(m_RendererID);
}

void OpenGLShader::Bind() const
{
	glUseProgram(m_RendererID);
}

void OpenGLShader::Unbind() const
{
	glUseProgram(0);
}

void OpenGLShader::SetInt(const std::string& name, int value)
{
	glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), value);
}

void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
{
	glUniform1iv(glGetUniformLocation(m_RendererID, name.c_str()), count, values);
}

void OpenGLShader::SetFloat(const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(m_RendererID, name.c_str()), value);
}

void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
{
	glUniform2f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y);
}

void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
{
	glUniform3f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y, value.z);
}

void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
{
	glUniform4f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y, value.z, value.w);
}

void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
{
	glUniformMatrix4fv(glGetUniformLocation(m_RendererID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void OpenGLShader::CheckCompileErrors(unsigned int shader, std::string type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			Logger::Log("shader compilation error", "Renderer", LOG_TYPE::CRITICAL);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			Logger::Log("shader program linking error", "Renderer", LOG_TYPE::CRITICAL);
		}
	}
}
