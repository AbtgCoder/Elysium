#pragma once

#include "Scene/ScriptableEntity.h"

class RotateEntity : public ScriptableEntity
{
public:
	void OnCreate() override;
	void OnDestroy() override;
	void OnUpdate(float ts) override;
};