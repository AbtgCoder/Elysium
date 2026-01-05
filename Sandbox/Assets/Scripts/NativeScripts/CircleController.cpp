#include "CircleController.h"

#include "core/Logger.h"

void CircleController::OnCreate()
{
	Logger::Log("oncreateeeeeeee!", "script");
}

void CircleController::OnDestroy()
{
	Logger::Log("on destroyyy!", "script");
}

void CircleController::OnUpdate(float ts)
{
	auto& transform = getComponent<CTransform>();
	transform.Rotation += 15.0f * ts;
	// transform.Translation.x += 10.0f;
}
