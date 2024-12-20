#pragma once
#include "core/Application.h"

extern Application* CreateApplication();

int main(int argc, char** argv)
{
	auto app = CreateApplication();
	app->run();
	delete app;
}