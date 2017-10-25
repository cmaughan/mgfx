#pragma once


namespace MediaType
{
enum : uint32_t
{
    Texture = (1 << 0),
    Model   = (1 << 1),
    Shader  = (1 << 2),
    Local   = (1 << 3),
    Document = (1 << 4),
    Project =  (1 << 5),
    Font = (1 << 6)
};
}

class MediaManager
{
public:
    static MediaManager& Instance();

    const MediaManager& operator= (const MediaManager& rhs) = delete;
    MediaManager(const MediaManager& rhs) = delete;

    bool SetAssetPath(const fs::path& asset);
    bool SetProjectPath(const fs::path& asset);

    const fs::path& GetProjectPath() { return m_projectPath; }

    fs::path GetDocumentsPath() { return m_documentsPath; }

    fs::path FindAsset(const char* pszAssetName, uint32_t type = MediaType::Document, const fs::path* assetBase = nullptr);
    std::string LoadAsset(const char* pszPath, uint32_t mediaType = MediaType::Document, const fs::path* assetBase = nullptr);

private:
    fs::path FindAssetInternal(const char* pszAssetName, uint32_t type, const fs::path* assetBase = nullptr);

    MediaManager();

    fs::path m_mediaPath;
    std::vector<fs::path> m_texturePaths;
    std::vector<fs::path> m_modelPaths;
    std::vector<fs::path> m_docPaths;
    std::vector<fs::path> m_shaderPaths;
    std::vector<fs::path> m_fontPaths;

    std::vector<std::string> m_textureExtensions;
    std::map<std::string, std::string> m_textureSubstringReplacements;

    std::map<std::string, fs::path> m_foundAssets;

    fs::path m_documentsPath;
    fs::path m_projectPath;
};