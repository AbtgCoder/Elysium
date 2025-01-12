#pragma once

#include "core/LayerStack.h"
#include "core/Window.h"

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

#include "ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

class Application
{
public:
	Application(const std::string& name);
	~Application();

	static Application& Get() { return *s_Instance; }

	void PushLayer(Layer* layer);
	void PushOverlay(Layer* layer);

	void OnEvent(Event& e);

	Window& GetWindow() { return *m_Window; }
	
	void Close();

	ImGuiLayer* GetImGuiLayer() { return  m_ImGuiLayer; }
private:
	void Run();
	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);
private:
	std::unique_ptr<Window> m_Window;
	ImGuiLayer* m_ImGuiLayer;
	float m_LastFrameTime = 0.0f;
	bool m_Running = true;
	LayerStack m_LayerStack;
private:
	static Application* s_Instance;
	friend int ::main(int argc, char** argv);
};

// to be defined in Client 
Application* CreateApplication();