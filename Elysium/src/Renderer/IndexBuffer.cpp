#include "Platform/OpenGL/OpenGLIndexBuffer.h"

#include "core/Logger.h"

#include "Renderer/RendererAPI.h"

std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
{
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::API::None:
		Logger::Log("RendererAPI::None is currently not supported", "Renderer", LOG_TYPE::CRITICAL);
		return nullptr;
	case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLIndexBuffer>(indices, count);
	}
}
