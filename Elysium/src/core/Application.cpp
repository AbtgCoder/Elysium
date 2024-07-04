#include "Application.h"
#include "Editor/EditorLayer.h"

#include "imgui.h"
#include "imgui-SFML.h"

Application::Application(const std::string& name)
{
	init(name);
}

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
	std::shared_ptr<Layer> editorLayer = std::make_shared<EditorLayer>(this);
	changeLayer("Editor_Layer", editorLayer, true);

}

void Application::update(float dt)
{
	currentLayer()->update(dt);
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
		float dt = deltaClock.restart().asSeconds();
		update(dt);
		m_window.display();
	}

	ImGui::SFML::Shutdown();

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


