#include "WindowsWindow.h"

static uint8_t s_GLFWWindowCount = 0;

WindowsWindow::WindowsWindow(const WindowProps& props)
{
	Init(props);
}

WindowsWindow::~WindowsWindow()
{
	Shutdown();
}


void WindowsWindow::Init(const WindowProps& props)
{
	m_Data.Title = props.Title;
	m_Data.Width = props.Width;
	m_Data.Height = props.Height;

	if (s_GLFWWindowCount == 0)
	{
		int success = glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // only use core-profile
		//glfwSetErrorCallback()
	}

	{
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		++s_GLFWWindowCount;
	}

	// create graphics(currently only opengl) context
	m_Context = GraphicsContext::Create(m_Window);
	m_Context->Init();

	glfwSetWindowUserPointer(m_Window, &m_Data);
	SetVSync(true);

	// set GLFW callbacks
}

void WindowsWindow::OnUpdate()
{
	glfwPollEvents();
	m_Context->SwapBuffers();
}


void WindowsWindow::Shutdown()
{
	glfwDestroyWindow(m_Window);
	--s_GLFWWindowCount;

	if (s_GLFWWindowCount == 0)
	{
		glfwTerminate();
	}
}

void WindowsWindow::SetVSync(bool enabled)
{
	if (enabled)
	{
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}

	m_Data.VSync = enabled;
}

bool WindowsWindow::IsVSync() const
{
	return m_Data.VSync;
}