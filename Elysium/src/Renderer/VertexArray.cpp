#include "core/Logger.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

#include "Renderer/RendererAPI.h"

std::shared_ptr<VertexArray> VertexArray::Create()
{
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::API::None:
		Logger::Log("RendererAPI::None is currently not supported", "Renderer", LOG_TYPE::CRITICAL);
		return nullptr;
	case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexArray>();
	}
}
