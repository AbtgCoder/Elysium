#pragma once

#include "Project/Project.h"
#include "Renderer/Texture.h"

struct ThumbnailImage
{
	uint64_t Timestamp;
	std::shared_ptr<Texture2D> Image;
};

class ThumbnailCache
{
public:
	ThumbnailCache(std::shared_ptr<Project> project);

	std::shared_ptr<Texture2D> GetOrCreateThumbnail(const std::filesystem::path& path);
private:
	std::shared_ptr<Project> m_Project;
	std::map<std::filesystem::path, ThumbnailImage> m_CachedImages;

	// temporary:
	std::filesystem::path m_ThumbnailCachePath;
};