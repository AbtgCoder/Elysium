#include "Renderer/GraphicsContext.h"

#include "Platform/OpenGL/OpenGLContext.h"

std::unique_ptr<GraphicsContext> GraphicsContext::Create(void* window)
{
	// if RendererAPI == OpenGL
	return std::make_unique<OpenGLContext>(static_cast<GLFWwindow*>(window));

}
