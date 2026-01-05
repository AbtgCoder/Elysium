#pragma once

#include "Scene/ScriptableEntity.h"

class CircleController : public ScriptableEntity
{
public:
	void OnCreate() override;
	void OnDestroy() override;
	void OnUpdate(float ts) override;
};

extern "C" __declspec(dllexport) ScriptableEntity* CreateScript()
{
	return new CircleController();
}
                                                         