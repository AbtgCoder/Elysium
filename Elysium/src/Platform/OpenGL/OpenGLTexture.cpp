#include "Platform/OpenGL/OpenGLTexture.h"


namespace Utils {
	static GLenum ImageFormatToGLDataFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8: return GL_RGB;
		case ImageFormat::RGBA8: return GL_RGBA;
		}

		return 0;
	}

	static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8: return GL_RGB8;
		case ImageFormat::RGBA8: return GL_RGBA8;
		}

		return 0;
	}
}



OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data)
	: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
{
	m_InternalFormat = Utils::ImageFormatToGLInternalFormat(m_Specification.Format);
	m_DataFormat = Utils::ImageFormatToGLDataFormat(m_Specification.Format);

	// opengl create texture

	glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
	glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

	// texture filtering methods
	glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// texture wrapping
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (data)
	{
		SetData(data);
	}
}

OpenGLTexture2D::~OpenGLTexture2D()
{
	glDeleteTextures(1, &m_RendererID);
}

void OpenGLTexture2D::SetData(Buffer data)
{
	uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
	
	glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
	/*
		this updates a specific part(based on xOffset, yOffset, width and height) of an existing texture's data without reallocating memory (as opposed
		to glTexImage2D), so it is also faster
		
		first_param: texture you want to update with new pixel data
		second_param: mip level
		third, fourth param: x and y offset
		width and height
		data format
		data type of the pixel data
		pixel data
	*/
}

void OpenGLTexture2D::Bind(uint32_t slot) const
{
	// for shader uniform sampler, put this texture at the given slot index
	// slot = 0 => GL_TEXTURE0 , slot = 1 => GL_TEXTURE1
	glBindTextureUnit(slot, m_RendererID);
}
