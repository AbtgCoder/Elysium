#include "Window.h"

#include "Platform/Windows/WindowsWindow.h"

std::unique_ptr<Window> Window::Create(const WindowProps& props)
{
	// if windows:
	return std::make_unique<WindowsWindow>(props);
}
