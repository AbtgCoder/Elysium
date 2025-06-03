#include "Renderer/Renderer2D.h"

#include "core/Logger.h"

#include "RenderCommand.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"

#include "Asset/AssetManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include <array>

struct QuadVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoord;
	float TexIndex;

	// editor-only
	int entityID;
};

struct LineVertex
{
	glm::vec3 Position;
	glm::vec4 Color;

	// editor-only
	int entityID;
};

struct Renderer2DData
{
	static const uint32_t MaxQuads = 20000;
	static const uint32_t MaxVertices = MaxQuads * 4;
	static const uint32_t MaxIndices = MaxQuads * 6;
	static const uint32_t MaxTextureSlots = 32;

	// Quads
	std::shared_ptr<VertexArray> QuadVertexArray;
	std::shared_ptr<VertexBuffer> QuadVertexBuffer;
	std::shared_ptr<Shader> QuadShader;
	glm::vec4 QuadVertexPositions[4];
	uint32_t QuadIndexCount = 0;
	QuadVertex* QuadVertexBufferBase = nullptr; // pointer to the start of array which will hold all quad vertices for a batch
	QuadVertex* QuadVertexBufferPtr = nullptr; // pointer into the QuadVertexBufferBase array

	// Lines
	std::shared_ptr<VertexArray> LineVertexArray;
	std::shared_ptr<VertexBuffer> LineVertexBuffer;
	std::shared_ptr<Shader> LineShader;
	uint32_t LineVertexCount = 0;
	LineVertex* LineVertexBufferBase = nullptr;
	LineVertex* LineVertexBufferPtr = nullptr;
	float LineWidth = 3.0f;
	
	// textures
	std::shared_ptr<Texture2D> WhiteTexture;
	std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
	uint32_t TextureSlotIndex = 1; // 0 = white texture
	float PixelsPerUnit = 100.0f; // for scaling the world to pixels(Unity uses this value ig)

	Renderer2D::Statistics Stats;
};

static Renderer2DData s_RenderData;

void Renderer2D::Init()
{
	// Quads
	s_RenderData.QuadVertexArray = VertexArray::Create();

	s_RenderData.QuadVertexBuffer = VertexBuffer::Create(s_RenderData.MaxVertices * sizeof(QuadVertex));
	s_RenderData.QuadVertexBuffer->SetLayout({
		{ShaderDataType::Float3, "a_Position"},		// dont need the names here for opengl but might need it for directX
		{ShaderDataType::Float4, "a_Color"},
		{ShaderDataType::Float2, "a_TexCoord"},
		{ShaderDataType::Float, "a_TexIndex"},
		{ShaderDataType::Int, "a_EntityID"}
		});
	s_RenderData.QuadVertexArray->AddVertexBuffer(s_RenderData.QuadVertexBuffer);

	s_RenderData.QuadIndexCount = 0;
	s_RenderData.QuadVertexBufferBase = new QuadVertex[s_RenderData.MaxVertices];
	s_RenderData.QuadVertexBufferPtr = s_RenderData.QuadVertexBufferBase;

	uint32_t* quadIndices = new uint32_t[s_RenderData.MaxIndices];

	uint32_t offset = 0;
	for (uint32_t i = 0; i < s_RenderData.MaxIndices; i += 6)
	{
		// first triangle
		quadIndices[i + 0] = offset + 0;
		quadIndices[i + 1] = offset + 1;
		quadIndices[i + 2] = offset + 2;
		// second triangle
		quadIndices[i + 3] = offset + 2;
		quadIndices[i + 4] = offset + 3;
		quadIndices[i + 5] = offset + 0;

		offset += 4;
	}

	std::shared_ptr<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, s_RenderData.MaxIndices);
	s_RenderData.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
	delete[] quadIndices;

	s_RenderData.QuadShader = Shader::Create("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Shaders/QuadVertexShader.glsl", "D:/Game Development/Game_Engine_Programming/Elysium/Resources/Shaders/QuadFragmentShader.glsl");

	s_RenderData.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	s_RenderData.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
	s_RenderData.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
	s_RenderData.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };


	// Lines
	s_RenderData.LineVertexArray = VertexArray::Create();

	s_RenderData.LineVertexBuffer = VertexBuffer::Create(s_RenderData.MaxVertices * sizeof(LineVertex));
	s_RenderData.LineVertexBuffer->SetLayout({
		{ShaderDataType::Float3, "a_Position"},
		{ShaderDataType::Float4, "a_Color"},
		{ShaderDataType::Int, "a_EntityID"},
		});
	s_RenderData.LineVertexArray->AddVertexBuffer(s_RenderData.LineVertexBuffer);
	
	s_RenderData.LineVertexBufferBase = new LineVertex[s_RenderData.MaxVertices];

	s_RenderData.LineShader = Shader::Create("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Shaders/LineVertexShader.glsl", "D:/Game Development/Game_Engine_Programming/Elysium/Resources/Shaders/LineFragmentShader.glsl");

	// textures
	s_RenderData.WhiteTexture = Texture2D::Create(TextureSpecification());
	uint32_t whiteTextureData = 0xffffffff; // all white texture
	s_RenderData.WhiteTexture->SetData(Buffer(&whiteTextureData, sizeof(uint32_t)));
	// set first texture slot to 0
	s_RenderData.TextureSlots[0] = s_RenderData.WhiteTexture;

	int32_t samplers[s_RenderData.MaxTextureSlots];
	for (uint32_t i = 0; i < s_RenderData.MaxTextureSlots; i++)
	{
		samplers[i] = i;
	}
	s_RenderData.QuadShader->Bind();
	s_RenderData.QuadShader->SetIntArray("u_Textures", samplers, 32);

}

void Renderer2D::Shutdown()
{
	delete[] s_RenderData.QuadVertexBufferBase;
	delete[] s_RenderData.LineVertexBufferBase;
}

void Renderer2D::BeginScene(const EditorCamera& camera)
{
	s_RenderData.QuadIndexCount = 0;
	s_RenderData.QuadVertexBufferPtr = s_RenderData.QuadVertexBufferBase;

	s_RenderData.LineVertexCount = 0;
	s_RenderData.LineVertexBufferPtr = s_RenderData.LineVertexBufferBase;

	s_RenderData.TextureSlotIndex = 1;

	s_RenderData.QuadShader->Bind();
	s_RenderData.QuadShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

	s_RenderData.LineShader->Bind();
	s_RenderData.LineShader->SetMat4("u_ViewProjection", camera.GetViewProjection());
}

void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
{
	s_RenderData.QuadIndexCount = 0;
	s_RenderData.QuadVertexBufferPtr = s_RenderData.QuadVertexBufferBase;

	s_RenderData.LineVertexCount = 0;
	s_RenderData.LineVertexBufferPtr = s_RenderData.LineVertexBufferBase;

	s_RenderData.TextureSlotIndex = 1;

	glm::mat4 viewProjection = camera.GetProjection() * glm::inverse(transform);
	
	s_RenderData.QuadShader->Bind();
	s_RenderData.QuadShader->SetMat4("u_ViewProjection", viewProjection);

	s_RenderData.LineShader->Bind();
	s_RenderData.LineShader->SetMat4("u_ViewProjection", viewProjection);
}

void Renderer2D::EndScene()
{
	Flush();
}

void Renderer2D::Flush()
{
	// batch render quads
	if (s_RenderData.QuadIndexCount)
	{
		uint32_t dataSize = (uint32_t)((uint8_t*)s_RenderData.QuadVertexBufferPtr - (uint8_t*)s_RenderData.QuadVertexBufferBase);
		s_RenderData.QuadVertexBuffer->SetData(s_RenderData.QuadVertexBufferBase, dataSize); // upload the quad vertex buffer array to the vertex buffer

		// bind textures
		for (uint32_t i = 0; i < s_RenderData.TextureSlotIndex; i++)
		{
			s_RenderData.TextureSlots[i]->Bind(i);
		}
		
		s_RenderData.QuadShader->Bind();
		RenderCommand::DrawIndexed(s_RenderData.QuadVertexArray, s_RenderData.QuadIndexCount);
		// update stats
		s_RenderData.Stats.DrawCalls++;
	}

	// batch render lines
	if (s_RenderData.LineVertexCount)
	{
		uint32_t dataSize = (uint32_t)((uint8_t*)s_RenderData.LineVertexBufferPtr - (uint8_t*)s_RenderData.LineVertexBufferBase);
		s_RenderData.LineVertexBuffer->SetData(s_RenderData.LineVertexBufferBase, dataSize);

		s_RenderData.LineShader->Bind();
		RenderCommand::SetLineWidth(s_RenderData.LineWidth);
		RenderCommand::DrawLines(s_RenderData.LineVertexArray, s_RenderData.LineVertexCount);
		// update stats
		s_RenderData.Stats.DrawCalls++;
	}
}


void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color,  int entityID)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f))
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
	
	DrawQuad(transform, color, entityID);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, const glm::vec4& tintColor, int entityID)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f))
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	DrawQuad(transform, texture, tintColor, entityID);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color, int entityID)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f))
		* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
	DrawQuad(transform, color, entityID);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
{
	constexpr size_t quadVertexCount = 4;
	const float textureIndex = 0.0f; // white texture
	constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
	
	for (size_t i = 0; i < quadVertexCount; i++)
	{
		// set the data for each quad vertex
		s_RenderData.QuadVertexBufferPtr->Position = transform * s_RenderData.QuadVertexPositions[i];
		s_RenderData.QuadVertexBufferPtr->Color = color;
		s_RenderData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
		s_RenderData.QuadVertexBufferPtr->TexIndex = textureIndex;

		// set entity id
		s_RenderData.QuadVertexBufferPtr->entityID = entityID;

		// update the pointer to the next position in the quad vertex array
		s_RenderData.QuadVertexBufferPtr++;
	}

	s_RenderData.QuadIndexCount += 6;

	// update stats
	s_RenderData.Stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D> texture, const glm::vec4& tintColor, int entityID)
{
	constexpr size_t quadVertexCount = 4;
	constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

	float textureIndex = 0.0f;
	for (uint32_t i = 1; i < s_RenderData.TextureSlotIndex; i++)
	{
		if (*s_RenderData.TextureSlots[i] == *texture) // we wont create another sampler if we are just using the same texture, we just upload it once and then use that texture wherever needed
		{
			textureIndex = (float)i;
			break;
		}
	}

	if (textureIndex == 0.0f)
	{
		textureIndex = (float)s_RenderData.TextureSlotIndex;
		s_RenderData.TextureSlots[s_RenderData.TextureSlotIndex] = texture;
		s_RenderData.TextureSlotIndex++;
	}

	//s_RenderData.TextureSlots[1] = texture;

	float texWidth = (float)texture->GetWidth();
	float texHeight = (float)texture->GetHeight();

	float worldWidth = texWidth / s_RenderData.PixelsPerUnit; // convert texture width to world units
	float worldHeight = texHeight / s_RenderData.PixelsPerUnit; // convert texture height to world units

	glm::mat4 scaledTransform = transform * glm::scale(glm::mat4(1.0f), glm::vec3(worldWidth, worldHeight, 1.0f));

	for (size_t i = 0; i < quadVertexCount; i++)
	{
		// set the data for each quad vertex
		s_RenderData.QuadVertexBufferPtr->Position = scaledTransform * s_RenderData.QuadVertexPositions[i];
		s_RenderData.QuadVertexBufferPtr->Color = tintColor;
		s_RenderData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
		s_RenderData.QuadVertexBufferPtr->TexIndex = textureIndex;

		// set entity id
		s_RenderData.QuadVertexBufferPtr->entityID = entityID;

		// update the pointer to the next position in the quad vertex array
		s_RenderData.QuadVertexBufferPtr++;
	}

	s_RenderData.QuadIndexCount += 6;

	// update stats
	s_RenderData.Stats.QuadCount++;
}

void Renderer2D::DrawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color, int entityID)
{
	s_RenderData.LineVertexBufferPtr->Position = glm::vec3(p1.x, p1.y, 0.0f);
	s_RenderData.LineVertexBufferPtr->Color = color;
	s_RenderData.LineVertexBufferPtr->entityID = entityID;
	s_RenderData.LineVertexBufferPtr++;

	s_RenderData.LineVertexBufferPtr->Position = glm::vec3(p2.x, p2.y, 0.0f);
	s_RenderData.LineVertexBufferPtr->Color = color;
	s_RenderData.LineVertexBufferPtr->entityID = entityID;
	s_RenderData.LineVertexBufferPtr++;

	s_RenderData.LineVertexCount += 2;
}

void Renderer2D::DrawRotatedRect(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color, int entityID)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f))
		* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
	DrawRect(transform, color, entityID);
}

void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
{
	glm::vec3 rectVertices[4];
	for (size_t i = 0; i < 4; i++)
		rectVertices[i] = transform * s_RenderData.QuadVertexPositions[i];

	DrawLine(rectVertices[0], rectVertices[1], color, entityID);
	DrawLine(rectVertices[1], rectVertices[2], color, entityID);
	DrawLine(rectVertices[2], rectVertices[3], color, entityID);
	DrawLine(rectVertices[3], rectVertices[0], color, entityID);

}

void Renderer2D::ResetStats()
{
	memset(&s_RenderData.Stats, 0, sizeof(Renderer2D::Statistics));
}

Renderer2D::Statistics Renderer2D::GetStats()
{
	return s_RenderData.Stats;
}

