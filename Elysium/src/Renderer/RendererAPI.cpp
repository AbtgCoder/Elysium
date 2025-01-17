#include "Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include "core/Logger.h"

RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

std::unique_ptr<RendererAPI> RendererAPI::Create()
{
	switch (s_API)
	{
	case RendererAPI::API::None: 
		Logger::Log("RendererAPI::None is currently not supported", "Renderer", LOG_TYPE::CRITICAL);
		return nullptr;
	case RendererAPI::API::OpenGL:  return std::make_unique<OpenGLRendererAPI>();
	}

	return nullptr;
}