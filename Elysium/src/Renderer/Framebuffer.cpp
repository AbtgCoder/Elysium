#include "Renderer/Framebuffer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include "core/Logger.h"

#include "Renderer/RendererAPI.h"

std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
{
	// only if RendererAPI is OpenGL
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::API::None:
		Logger::Log("RendererAPI::None is currently not supported", "Renderer", LOG_TYPE::CRITICAL);
		return nullptr;
	case RendererAPI::API::OpenGL:
		return std::make_shared<OpenGLFramebuffer>(spec);
	}
}
