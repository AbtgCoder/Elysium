#pragma once

#include "Renderer/EditorCamera.h"

#include "Renderer/Texture.h"

class Renderer2D
{
public:
	static void Init();
	static void Shutdown();

	static void BeginScene(const EditorCamera& camera);
	static void BeginScene(const Camera& camera, const glm::mat4& transform);
	static void EndScene();
	static void Flush();

	// primitives
	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);
	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

	static void DrawQuad(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);
	static void DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D> texture, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);
	static void DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D> texture, const glm::vec4& tintColor = glm::vec4(1.0f), const glm::vec2& uvMin = glm::vec2(0.0f, 0.0f), const glm::vec2& uvMax = glm::vec2(1.0f, 1.0f), int entityID = -1);


	static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

	static void DrawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

	static void DrawRotatedRect(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);
	static void DrawRect(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

public:
	// stats
	struct Statistics
	{
		uint32_t DrawCalls = 0;
		uint32_t QuadCount = 0;

		uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
		uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
	};

	static void ResetStats();
	static Statistics GetStats();
};