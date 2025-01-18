#include "Renderer/Renderer2D.h"

#include "Renderer/Shader.h"

#include <glm/glm.hpp>

#include <glad/glad.h>

struct Renderer2DData
{
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	std::shared_ptr<Shader> triangleShader;

	float triangleVertices[9] = {
		-0.5f, -0.5f, 0.0f, // left  
		 0.5f, -0.5f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f  // top   
	};
};

static Renderer2DData s_RenderData;

void Renderer2D::Init()
{
	glGenVertexArrays(1, &s_RenderData.VAO);
	glGenBuffers(1, &s_RenderData.VBO);

	glBindVertexArray(s_RenderData.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, s_RenderData.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_RenderData.triangleVertices), s_RenderData.triangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	s_RenderData.triangleShader = Shader::Create("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Shaders/triangleVertexShader.glsl", "D:/Game Development/Game_Engine_Programming/Elysium/Resources/Shaders/triangleFragmentShader.glsl");
}

void Renderer2D::Shutdown()
{
}

void Renderer2D::BeginScene(const EditorCamera& camera)
{
	s_RenderData.triangleShader->Bind();
	s_RenderData.triangleShader->SetMat4("u_ViewProjection", camera.GetViewProjection());
}

void Renderer2D::EndScene()
{
}

void Renderer2D::DrawTriangle()
{
	s_RenderData.triangleShader->Bind();
	glBindVertexArray(s_RenderData.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
