#include "ThumbnailCache.h"

#include "Asset/TextureImporter.h"

#include <chrono>

ThumbnailCache::ThumbnailCache(std::shared_ptr<Project> project)
	: m_Project(project)
{
	//TODO: revisit this path(move to cache dir)
	m_ThumbnailCachePath = m_Project->GetAssetDirectory() / "Thumbnail.cache";
}

std::shared_ptr<Texture> ThumbnailCache::GetOrCreateThumbnail(const std::filesystem::path& path)
{
	// 1. Read file timestamp
	// 2. Compare hashed timestamp with existing cached image (in memory first,TODO: then from cache file)
	// 3. If equal, return associated thumbnail, otherwise load asset from disk and generate thumbnail
	// 4. If generated new thumbnail, store in cache obviously

	auto absolutePath = path; //TODO: change this if we recieve the relative path
	std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(absolutePath);
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count();

	if (m_CachedImages.find(path) != m_CachedImages.end())
	{
		auto& cachedImage = m_CachedImages.at(path);
		if (cachedImage.Timestamp == timestamp)
			return cachedImage.Image;
	}
	
	//TODO: support other extensions...
	if (path.extension() != ".png")
		return nullptr;

	std::shared_ptr<Texture> texture = TextureImporter::LoadTexture(absolutePath);
	if (!texture)
		return nullptr;

	auto& cachedImage = m_CachedImages[path];
	cachedImage.Timestamp = timestamp;
	cachedImage.Image = texture;
	return cachedImage.Image;
}
