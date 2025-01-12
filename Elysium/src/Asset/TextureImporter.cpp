#include "TextureImporter.h"

#include "Project/Project.h"

#include "core/Logger.h"

#include <stb_image.h>

std::shared_ptr<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
{
	return LoadTexture2D(Project::GetActiveAssetDirectory() / metadata.FilePath);
}

std::shared_ptr<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& path)
{
	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);

	Buffer data;
	{
		std::string pathStr = path.string();
		data.Data = stbi_load(pathStr.c_str(), &width, &height, &channels, 4);
		channels = 4;
	}

	if (data.Data == nullptr)
	{
		// couldnt load texture from filepath
		return nullptr;
	}

	data.Size = width * height * channels;

	TextureSpecification spec;
	spec.Width = width;
	spec.Height = height;
	switch (channels)
	{
	case 3:
		spec.Format = ImageFormat::RGB8;
		break;
	case 4:
		spec.Format = ImageFormat::RGBA8;
		break;
	}

	std::shared_ptr<Texture2D> texture = Texture2D::Create(spec, data);
	data.Release();
	return texture;
}
