#include "Texture.h"

#include "Platform/OpenGL/OpenGLTexture.h"

std::shared_ptr<Texture2D> Texture2D::Create(const TextureSpecification& specification, Buffer data)
{
	// if using RenderingAPI == OpenGL
	return std::make_shared<OpenGLTexture2D>(specification, data);
}
