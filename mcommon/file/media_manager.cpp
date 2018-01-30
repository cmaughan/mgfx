#include "mcommon.h"
#include "file/media_manager.h"
#include "file/fileutils.h"
#include "config_app.h"

std::string g_AppFriendlyName = APPLICATION_NAME;

MediaManager& MediaManager::Instance()
{
    static MediaManager manager;
    return manager;
}

bool MediaManager::SetAssetPath(const fs::path& assetPath)
{
    m_documentsPath = FileUtils::GetDocumentsPath() / fs::path(g_AppFriendlyName);
    m_mediaPath = fs::canonical(fs::absolute(assetPath));

    LOG(INFO) << "Media Path: " << m_mediaPath.string();

    m_texturePaths.push_back(fs::path("textures"));
    m_shaderPaths.push_back(fs::path("shaders"));
    m_shaderPaths.push_back(fs::path("shaders") / fs::path("GL"));
    m_shaderPaths.push_back(fs::path("shaders") / fs::path("DX12"));
    m_modelPaths.push_back(fs::path("models"));
    m_docPaths.push_back(fs::path("documents"));
    m_modelPaths.push_back(fs::path("test") / fs::path("models"));

    m_fontPaths.push_back(fs::path("fonts"));

    m_textureExtensions.push_back(".dds");
    m_textureExtensions.push_back(".png");

    m_textureSubstringReplacements["_bump"] = "_normal";
    return true;
}

bool MediaManager::SetProjectPath(const fs::path& project)
{
    if (fs::exists(project))
    {
        m_projectPath = fs::canonical(fs::absolute(project));
        return true;
    }
    
    try
    {
        fs::path newPath;
        if (project.is_relative())
        {
            newPath = m_documentsPath / project;
        }
        else
        {
            newPath = project;
        }

        if (!fs::exists(newPath))
        {
            if (!fs::create_directories(newPath))
            {
                return false;
            }
        }
        m_projectPath = fs::canonical(fs::absolute(newPath));
        return true;
    }
    catch (fs::filesystem_error& err)
    {
        LOG(ERROR) << err.what();
    }
    return false;
}

MediaManager::MediaManager()
{
}

std::string MediaManager::LoadAsset(const char* pszPath, uint32_t mediaType, const fs::path* assetBase)
{
    auto path = MediaManager::Instance().FindAsset(pszPath, mediaType, assetBase);
    return FileUtils::ReadFile(path);
}

fs::path MediaManager::FindAsset(const char* pszPath, uint32_t mediaType, const fs::path* assetBase)
{
    auto itrFound = m_foundAssets.find(pszPath);
    if (itrFound != m_foundAssets.end())
    {
        return itrFound->second;
    }

    fs::path found = FindAssetInternal(pszPath, mediaType, assetBase);
    if (!found.empty())
    {
        m_foundAssets[pszPath] = found;
    }
    return found;
}

fs::path MediaManager::FindAssetInternal(const char* pszPath, uint32_t mediaType, const fs::path* assetBase)
{
    fs::path searchPath(pszPath);
    fs::path parent;

    if (assetBase)
    {
        parent = *assetBase;
    }

    // If an absolute path, start by checking it points at something
    if (searchPath.is_absolute())
    {
        if (fs::exists(searchPath))
        {
            // Return clean path
            LOG(DEBUG) << "Found path: " << searchPath.string();
            return fs::canonical(searchPath);
        }

        searchPath = searchPath.filename();

        // Absolute parent the default if not specified
        if (parent.empty())
        {
            parent = searchPath.parent_path();
        }
    }

    if (parent.empty())
    {
        // Otherwise, start at the base of the media path
        parent = m_mediaPath;
    }

    auto searchPaths = [&](const fs::path& parentPath, const fs::path& fileSearch, const std::vector<fs::path>& searchPaths)
    {
        for (auto& currentSearch : searchPaths)
        {
            fs::path testPath = parentPath / currentSearch / fileSearch;
            if (fs::exists(testPath))
            {
                return fs::canonical(fs::absolute(testPath));
            }
        }

        fs::path parentChild = parentPath / fileSearch;
        if (fs::exists(parentChild))
        {
            return fs::canonical(fs::absolute(parentChild));
        }

        return fs::path();
    };

    if (mediaType & MediaType::Local)
    {
        fs::path found(searchPath);
        if (fs::exists(found))
        {
            LOG(DEBUG) << "Found local file: " << found.string();
            return fs::canonical(fs::absolute(found));
        }
    }


    if (mediaType & MediaType::Texture)
    {
        std::vector<fs::path> testFiles;
        testFiles.push_back(searchPath);

        for (auto& extension : m_textureExtensions)
        {
            testFiles.push_back(fs::path(searchPath).replace_extension(extension));
        }

        size_t index = 0;
        size_t lastIndex = testFiles.size();
        while(index < lastIndex)
        {
            for (auto& replacement : m_textureSubstringReplacements)
            {
                if (testFiles[index].string().find(replacement.first) != std::string::npos)
                {
                    testFiles.push_back(StringUtils::ReplaceString(testFiles[index].string(), replacement.first, replacement.second));
                }
            }
            index++;
        }

        for (auto& current : testFiles)
        {
            fs::path found = searchPaths(parent, current, m_texturePaths);
            if (!found.empty())
            {
                LOG(DEBUG) << "Found texture: " << found.string();
                return found;
            }
        }

        LOG(DEBUG) << "Texture not found, searches: ";
        for (auto& current : testFiles)
        {
            LOG(DEBUG) << current.string();
        }
    }

    if (mediaType & MediaType::Model)
    {
        fs::path found = searchPaths(parent, searchPath, m_modelPaths);
        if (!found.empty())
        {
            LOG(DEBUG) << "Found model: " << found.string();
            return found;
        }
    }
    
    if (mediaType & MediaType::Document)
    {
        fs::path found = searchPaths(parent, searchPath, m_docPaths);
        if (!found.empty())
        {
            LOG(DEBUG) << "Found document: " << found.string();
            return found;
        }
    }

    if (mediaType & MediaType::Font)
    {
        fs::path found = searchPaths(parent, searchPath, m_fontPaths);
        if (!found.empty())
        {
            LOG(DEBUG) << "Found font: " << found.string();
            return found;
        }
    }
    if (mediaType & MediaType::Shader)
    {
        fs::path found = searchPaths(parent, searchPath, m_shaderPaths);
        if (!found.empty())
        {
            LOG(DEBUG) << "Found shader: " << found.string();
            return found;
        }
    }

    LOG(DEBUG) << "** File not found: " << searchPath.string();
    return fs::path();
}

