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


#if 0

void Application::init(const std::string& name)
{
	// create window
	m_window.create(sf::VideoMode::getDesktopMode(), name);
	m_window.setFramerateLimit(60);

	// Initialize ImGui
	ImGui::SFML::Init(m_window);
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // enable multi viewport, (only work on windows??)

	// Using Custom icon font with ImGui
#if 0
	ImFontConfig config;
	config.MergeMode = true;
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFont* font = io.Fonts->AddFontFromFileTTF("../../../Assets/fonts/fa-regular.ttf", 8.0f, &config, icon_ranges);
	//io.Fonts->Build();
	ImGui::SFML::UpdateFontTexture();
#endif

	// Initialize Editor Layer
	//std::shared_ptr<Layer> editorLayer = std::make_shared<EditorLayer>(this);
	//changeLayer("Editor_Layer", editorLayer, true);

}

void Application::update(float ts)
{
	currentLayer()->update(ts);
}

std::shared_ptr<Layer> Application::currentLayer()
{
	return m_LayerMap[m_currentLayer];
}

sf::RenderWindow& Application::window()
{
	return m_window;
}

bool Application::isRunning()
{
	return m_running;
}

const LayerMap& Application::Layers() const
{
	return m_LayerMap;
}




void Application::changeLayer(const std::string& LayerName, std::shared_ptr<Layer> Layer, bool endCurrentLayer)
{
	if (endCurrentLayer)
	{
		m_LayerMap.erase(m_currentLayer);
	}
	
	m_currentLayer = LayerName;
	m_LayerMap[m_currentLayer] = Layer;
}


void Application::quit()
{
	m_running = false;
}

void Application::run()
{
	// main game loop
	sf::Clock deltaClock;
	while (m_running && m_window.isOpen())
	{
		sUserInput();
		float ts = deltaClock.restart().asSeconds(); // Timestep ts
		ImGui::SFML::Update(m_window, m_deltaClock.restart());
		update(ts);
		ImGui::SFML::Render(m_window);
		m_window.display();
	}

	//ImGui::SFML::Shutdown();

}

void Application::sUserInput()
{
	// handle user input

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		/*if (ImGui::GetIO().WantCaptureMouse)
			continue;*/

		if (event.type == sf::Event::Closed)
		{
			m_running = false; 
			currentLayer()->sDoAction(Action("QUIT", "START"));
			//m_window.close();
		}

		if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
		{
			// if current Layer has actions
			if (currentLayer()->getActionMap().find(event.key.code) == currentLayer()->getActionMap().end())
			{
				continue;
			}

			// determine start or end of action
			const std::string actionType = (event.type == sf::Event::KeyPressed) ? "START" : "END";
			currentLayer()->sDoAction(Action(currentLayer()->getActionMap().at(event.key.code), actionType));

		}

		auto mpos = sf::Mouse::getPosition(m_window);
		Vec2 pos((float)mpos.x, (float)mpos.y);
		if (event.type == sf::Event::MouseButtonPressed)
		{
			switch (event.mouseButton.button)
			{
			case sf::Mouse::Left: { currentLayer()->sDoAction(Action("LEFT_CLICK", "START", pos)); break; }
			case sf::Mouse::Right: { currentLayer()->sDoAction(Action("RIGHT_CLICK", "START", pos)); break; }
			case sf::Mouse::Middle: { currentLayer()->sDoAction(Action("MIDDLE_CLICK", "START", pos)); break; }
			default: break;
			}
		}
		if (event.type == sf::Event::MouseButtonReleased)
		{
			switch (event.mouseButton.button)
			{
			case sf::Mouse::Left: { currentLayer()->sDoAction(Action("LEFT_CLICK", "END", pos)); break; }
			case sf::Mouse::Right: { currentLayer()->sDoAction(Action("RIGHT_CLICK", "END", pos)); break; }
			case sf::Mouse::Middle: { currentLayer()->sDoAction(Action("MIDDLE_CLICK", "END", pos)); break; }
			default: break;
			}
		}
		if (event.type == sf::Event::MouseMoved)
		{
			currentLayer()->sDoAction(Action("MOUSE_MOVE", Vec2((float)event.mouseMove.x, (float)event.mouseMove.y)));
		}
		if (event.type == sf::Event::MouseWheelScrolled)
		{
			currentLayer()->sDoAction(Action("MOUSE_WHEEL_SCROLL", Vec2(event.mouseWheelScroll.delta, event.mouseWheelScroll.delta)));
		}
	}
}

#endif

