#pragma once

#include "Level.h"
#include <filesystem>

class LevelSerializer
{
public:
	LevelSerializer(const std::shared_ptr<Level>& level);

	void Serialize(const std::filesystem::path& filepath);

	bool Deserialize(const std::filesystem::path& filepath);
private:
	std::shared_ptr<Level> m_Level;
};