
#include "Renderer/RendererAPI.h"

#include "core/Logger.h"

#include "Platform/OpenGL/OpenGLVertexBuffer.h"

std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size)
{
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::API::None:
		Logger::Log("RendererAPI::None is currently not supported", "Renderer", LOG_TYPE::CRITICAL);
		return nullptr;
	case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexBuffer>(size);
	}
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
{
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::API::None:
		Logger::Log("RendererAPI::None is currently not supported", "Renderer", LOG_TYPE::CRITICAL);
		return nullptr;
	case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexBuffer>(vertices, size);
	}
}
