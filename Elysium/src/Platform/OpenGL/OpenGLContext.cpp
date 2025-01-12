#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include <iostream>

OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
	: m_WindowHandle(windowHandle)
{
}

void OpenGLContext::Init()
{
	glfwMakeContextCurrent(m_WindowHandle);
	
	// load glad
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	if (!status)
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

}

void OpenGLContext::SwapBuffers()
{
	glfwSwapBuffers(m_WindowHandle);
}
