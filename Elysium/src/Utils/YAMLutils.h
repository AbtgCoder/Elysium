#pragma once

#include <yaml-cpp/yaml.h>

#include "Core/UUID.h"
#include "Math/Vec2.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs);
		static bool decode(const Node & node, glm::vec2 & rhs);
	};

	template<>
	struct convert<Vec2>
	{
		static Node encode(const Vec2& rhs);
		static bool decode(const Node& node, Vec2& rhs);
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs);
		static bool decode(const Node& node, glm::vec3& rhs);
	};

	template<>
	struct convert<Elysium::UUID>
	{
		static Node encode(const Elysium::UUID& uuid);
		static bool decode(const Node& node, Elysium::UUID& uuid);
	};
}

YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec2& v);

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec2& v);

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v);
