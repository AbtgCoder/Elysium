#include "Renderer/Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"

std::shared_ptr<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
{
	return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
}

std::shared_ptr<Shader> Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
{
	return std::make_shared<OpenGLShader>(vertexSrc, fragmentSrc);
}

void ShaderLibrary::Add(const std::string& name, const std::shared_ptr<Shader>& shader)
{
	if (!Exists(name))
	{
		m_Shaders[name] = shader;
	}
}

void ShaderLibrary::Add(const std::shared_ptr<Shader>& shader)
{
	auto& name = shader->GetName();
	Add(name, shader);
}

std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& vertexPath, const std::string& fragmentPath)
{
	auto shader = Shader::Create(vertexPath, fragmentPath);
	Add(shader);
	return shader;
}

std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
{
	auto shader = Shader::Create(name, vertexPath, fragmentPath);
	Add(name, shader);
	return shader;
}

std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& name)
{
	if (Exists(name))
	{
		return m_Shaders[name];
	}
	return nullptr;
}

bool ShaderLibrary::Exists(const std::string& name) const
{
	return m_Shaders.find(name) != m_Shaders.end();
}
