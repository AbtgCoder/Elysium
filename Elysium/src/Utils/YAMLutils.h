#pragma once

#include <yaml-cpp/yaml.h>

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
}

YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec2& v);