#pragma once

#include "Project/Project.h"
#include "core/Texture.h"

struct ThumbnailImage
{
	uint64_t Timestamp;
	std::shared_ptr<Texture> Image;
};

class ThumbnailCache
{
public:
	ThumbnailCache(std::shared_ptr<Project> project);

	std::shared_ptr<Texture> GetOrCreateThumbnail(const std::filesystem::path& path);
private:
	std::shared_ptr<Project> m_Project;
	std::map<std::filesystem::path, ThumbnailImage> m_CachedImages;

	// temporary:
	std::filesystem::path m_ThumbnailCachePath;
};