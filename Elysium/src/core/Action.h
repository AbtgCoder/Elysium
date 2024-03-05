#pragma once

#include <string>

#include "Math/Vec2.h"


class Action
{
public:
	const std::string& name() const;
	const std::string& type() const;
	const Vec2& pos() const;
	Action();
	Action(const std::string& name, const std::string& type);
	Action(const std::string& name, const std::string& type, const Vec2& pos);
	Action(const std::string& name, const Vec2& pos);

private:
	std::string m_name;
	std::string m_type;
	Vec2 m_pos;
};