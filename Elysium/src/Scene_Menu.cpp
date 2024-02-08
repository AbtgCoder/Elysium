#include "Scene_Menu.h"
#include "GameEngine.h"
#include "Scene_Play.h"
#include "LevelEditor.h"
#include "imgui.h"
#include "imconfig-SFML.h"

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

void Scene_Menu::init()
{
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::Num1, "LEVEL1");
	registerAction(sf::Keyboard::Num2, "LEVEL2");
	registerAction(sf::Keyboard::Num3, "LEVEL_EDITOR");



	m_levelPaths.push_back("../../../assets/levels/level_test.txt");
	m_levelPaths.push_back("../../../assets/levels/level_test2.txt");

	m_menuText.setCharacterSize(36);
	m_menuText.setFont(m_game->assets().getFont("Tech"));
	m_menuText.setString("Mega Mario");
	m_menuText.setFillColor(sf::Color::White);
}

void Scene_Menu::update()
{
	sRender();
}

void Scene_Menu::onEnd()
{
	if (m_hasEnded)
	{
		m_game->quit();
	}
	else
	{
		std::string levelPath = m_levelPaths[m_selectedMenuIndex];
		std::shared_ptr<Scene> scenePlay = std::make_shared<Scene_Play>(m_game, levelPath);
		m_game->changeScene("play", scenePlay, true);
	}
}

void Scene_Menu::sRender()
{
	sf::RenderWindow& window = m_game->window();
	window.clear(sf::Color(100, 100, 255));
	window.draw(m_menuText);
}

void Scene_Menu::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "QUIT")
		{
			m_hasEnded = true;
			onEnd();
		}
		else if (action.name() == "LEVEL1")
		{
			m_selectedMenuIndex = 0;
			onEnd();
		}
		else if (action.name() == "LEVEL2")
		{
			m_selectedMenuIndex = 1;
			onEnd();
		}
		else if (action.name() == "LEVEL_EDITOR")
		{
			std::shared_ptr<Scene> levelEditor = std::make_shared<LevelEditor>(m_game);
			m_game->changeScene("level_editor", levelEditor, true);
		}
	}

}
