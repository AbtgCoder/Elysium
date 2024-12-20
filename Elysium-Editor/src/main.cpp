#include "Core/EntryPoint.h"

#include "EditorLayer.h"


class Editor : public Application
{
public:
	Editor(const std::string& name)
		: Application(name)
	{
		changeLayer("Editor_Layer", std::make_shared<EditorLayer>(), true);
	}
};

Application* CreateApplication()
{
	return new Editor("Elysium");
}

