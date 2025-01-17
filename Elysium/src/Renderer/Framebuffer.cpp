#include "Renderer/Framebuffer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
{
	// only if RendererAPI is OpenGL
	return std::make_shared<OpenGLFramebuffer>(spec);
}
