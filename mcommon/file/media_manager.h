#pragma once

namespace MediaType
{
enum : uint32_t
{
    Texture = (1 << 0),
    Model   = (1 << 1),
    Shader  = (1 << 2),
    All = (0xFFFFFFFF)
};
}

class MediaManager
{
public:
    static MediaManager& Instance();

    const MediaManager& operator= (const MediaManager& rhs) = delete;
    MediaManager(const MediaManager& rhs) = delete;

    fs::path FindAsset(const char* pszAssetName, uint32_t type = MediaType::All, const fs::path* assetBase = nullptr);
    std::string LoadAsset(const char* pszPath, uint32_t mediaType = MediaType::All, const fs::path* assetBase = nullptr);

private:
    fs::path FindAssetInternal(const char* pszAssetName, uint32_t type = MediaType::All, const fs::path* assetBase = nullptr);

    MediaManager();

    fs::path m_mediaPath;
    std::vector<fs::path> m_texturePaths;
    std::vector<fs::path> m_modelPaths;
    std::vector<fs::path> m_shaderPaths;

    std::vector<std::string> m_textureExtensions;
    std::map<std::string, std::string> m_textureSubstringReplacements;

    std::map<std::string, fs::path> m_foundAssets;
};