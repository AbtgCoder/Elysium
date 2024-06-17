#include "GameEngine.h"
#include "Editor/LevelEditor.h"

#include "imgui.h"
#include "imgui-SFML.h"

GameEngine::GameEngine(const std::string& name)
{
	init(name);
}

void GameEngine::init(const std::string& name)
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

	// Initialize Level Editor Scene
	std::shared_ptr<Scene> levelEditor = std::make_shared<LevelEditor>(this);
	changeScene("level_editor", levelEditor, true);

}

void GameEngine::update(float dt)
{
	currentScene()->update(dt);
}

std::shared_ptr<Scene> GameEngine::currentScene()
{
	return m_sceneMap[m_currentScene];
}

sf::RenderWindow& GameEngine::window()
{
	return m_window;
}

bool GameEngine::isRunning()
{
	return m_running;
}

const SceneMap& GameEngine::scenes() const
{
	return m_sceneMap;
}



void GameEngine::changeScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene)
{
	if (endCurrentScene)
	{
		m_sceneMap.erase(m_currentScene);
	}
	
	m_currentScene = sceneName;
	m_sceneMap[m_currentScene] = scene;
}


void GameEngine::quit()
{
	m_running = false;
}

void GameEngine::run()
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

void GameEngine::sUserInput()
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
			// if current scene has actions
			if (currentScene()->getActionMap().find(event.key.code) == currentScene()->getActionMap().end())
			{
				continue;
			}

			// determine start or end of action
			const std::string actionType = (event.type == sf::Event::KeyPressed) ? "START" : "END";
			currentScene()->sDoAction(Action(currentScene()->getActionMap().at(event.key.code), actionType));

		}

		auto mpos = sf::Mouse::getPosition(m_window);
		Vec2 pos((float)mpos.x, (float)mpos.y);
		if (event.type == sf::Event::MouseButtonPressed)
		{
			switch (event.mouseButton.button)
			{
			case sf::Mouse::Left: { currentScene()->sDoAction(Action("LEFT_CLICK", "START", pos)); break; }
			case sf::Mouse::Right: { currentScene()->sDoAction(Action("RIGHT_CLICK", "START", pos)); break; }
			case sf::Mouse::Middle: { currentScene()->sDoAction(Action("MIDDLE_CLICK", "START", pos)); break; }
			default: break;
			}
		}
		if (event.type == sf::Event::MouseButtonReleased)
		{
			switch (event.mouseButton.button)
			{
			case sf::Mouse::Left: { currentScene()->sDoAction(Action("LEFT_CLICK", "END", pos)); break; }
			case sf::Mouse::Right: { currentScene()->sDoAction(Action("RIGHT_CLICK", "END", pos)); break; }
			case sf::Mouse::Middle: { currentScene()->sDoAction(Action("MIDDLE_CLICK", "END", pos)); break; }
			default: break;
			}
		}
		if (event.type == sf::Event::MouseMoved)
		{
			currentScene()->sDoAction(Action("MOUSE_MOVE", Vec2((float)event.mouseMove.x, (float)event.mouseMove.y)));
		}
		if (event.type == sf::Event::MouseWheelScrolled)
		{
			currentScene()->sDoAction(Action("MOUSE_WHEEL_SCROLL", Vec2(event.mouseWheelScroll.delta, event.mouseWheelScroll.delta)));
		}
	}
}


