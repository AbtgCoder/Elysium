#include "Application.h"
#include "Project/Project.h"

#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer2D.h"

#include "core/Logger.h"

#include <GLFW/glfw3.h>

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
{
	s_Instance = this;

	m_Window = Window::Create(WindowProps(name));
	m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	
	// initialize renderer
	RenderCommand::Init();
	Renderer2D::Init();

	// create and push imgui layer
	m_ImGuiLayer = new ImGuiLayer();
	PushOverlay(m_ImGuiLayer);

}

Application::~Application()
{
	Renderer2D::Shutdown();
}

void Application::PushLayer(Layer* layer)
{
	m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void Application::PushOverlay(Layer* layer)
{
	m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

void Application::OnEvent(Event& e)
{
	//Logger::Log(e.ToString());

	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	{
		if (e.Handled)
		{
			break;
		}
		(*it)->OnEvent(e);
	}
}

void Application::Close()
{
	m_Running = false;
}

void Application::Run()
{
	while (m_Running)
	{
		float time = glfwGetTime();
		float timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

		for (Layer* layer : m_LayerStack)
		{
			layer->OnUpdate(timestep);
		}

		m_ImGuiLayer->Begin();
		{
			for (Layer* layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
		}
		m_ImGuiLayer->End();

		m_Window->OnUpdate();
	}
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
	RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());

	return false;
}

