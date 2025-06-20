#include "Core/EntryPoint.h"

#include "EditorLayer.h"


class Editor : public Application
{
public:
	Editor(const std::string& name)
		: Application(name)
	{
		PushLayer(new EditorLayer());
	}
};

Application* CreateApplication()
{
	return new Editor("Elysium");
}

