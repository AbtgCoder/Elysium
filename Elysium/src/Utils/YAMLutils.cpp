#include "YAMLutils.h"

namespace YAML
{
	Node convert<glm::vec2>::encode(const glm::vec2& rhs)
	{
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.SetStyle(EmitterStyle::Flow);
		return node;
	}

	bool convert<glm::vec2>::decode(const Node& node, glm::vec2& rhs)
	{
		if (!node.IsSequence() || node.size() != 2)
			return false;

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		return true;
	}

	Node convert<Vec2>::encode(const Vec2& rhs)
	{
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.SetStyle(EmitterStyle::Flow);
		return node;
	}

	bool convert<Vec2>::decode(const Node& node, Vec2& rhs)
	{
		if (!node.IsSequence() || node.size() != 2)
			return false;
		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		return true;
	}

	Node convert<glm::vec3>::encode(const glm::vec3& rhs)
	{
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		node.SetStyle(EmitterStyle::Flow);
		return node;
	}

	bool convert<glm::vec3>::decode(const Node& node, glm::vec3& rhs)
	{
		if (!node.IsSequence() || node.size() != 3)
			return false;
		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		return true;
	}

	Node convert<Elysium::UUID>::encode(const Elysium::UUID& uuid)
	{
		Node node;
		node.push_back((uint64_t)uuid);
		return node;
	}

	bool convert<Elysium::UUID>::decode(const Node& node, Elysium::UUID& uuid)
	{
		uuid = node.as<uint64_t>();
		return true;
	}
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}