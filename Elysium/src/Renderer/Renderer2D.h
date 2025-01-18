#pragma once

#include "Renderer/EditorCamera.h"

class Renderer2D
{
public:
	static void Init();
	static void Shutdown();

	static void BeginScene(const EditorCamera& camera);
	static void EndScene();
	
	// primitives
	static void DrawTriangle();
};