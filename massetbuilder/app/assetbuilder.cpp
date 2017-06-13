#include "massetbuilder_app.h"
#include "tinydir/tinydir.h"
#include "json/src/json.hpp"
#include "file/fileutils.h"

#include "assetbuilder.h"

#include <queue>
#include <set>

#if PROJECT_DEVICE_DX12
#include "dxcompiler.h"
#endif

using namespace nlohmann;

namespace MAssetBuilder 
{


void to_json(json& j, const BuildArtifact& a)
{
    json artifacts;
    for (auto& artifact : a.outputs)
    {
        artifacts.push_back(artifact.string());
    }

    j["outputDir"] = a.outputDir.string();
    j["sourceFile"] = a.sourceFile.string();
    j["sourceFileMeta"] = a.sourceFileMeta.string();
    j["sourceFileTime"] = std::chrono::system_clock::to_time_t(a.sourceFileTime);
    j["outputs"] = artifacts;
    j["success"] = a.success;
}

void from_json(const nlohmann::json& j, BuildArtifact& a)
{
    a.outputDir = j["outputDir"].get<std::string>();
    a.sourceFile = j["sourceFile"].get<std::string>();
    a.sourceFileMeta = j["sourceFileMeta"].get<std::string>();
    for (auto& output : j["outputs"])
    {
        a.outputs.push_back(output.get<std::string>());
    }
    a.sourceFileTime = std::chrono::system_clock::from_time_t(j["sourceFileTime"]);
    a.success = j["success"];
}

AssetBuilder::AssetBuilder()
{

}

void AssetBuilder::GatherAssets()
{
    if (MAssetBuilderSettings::Instance().GetVerbose())
    {
        LOG(INFO) << "Gathering assets...";
    }

    m_assets.clear();

    tinydir_dir dir;
    if (tinydir_open(&dir, MAssetBuilderSettings::Instance().GetInputPath().string().c_str()) == -1)
    {
        LOG(INFO) << "Asset Path Invalid: " << MAssetBuilderSettings::Instance().GetInputPath().string();
        return;
    }

    fs::path rootPath = MAssetBuilderSettings::Instance().GetInputPath();
    rootPath = fs::canonical(fs::absolute(rootPath));

    std::set<fs::path> checkedPaths;

    // Walk recursively down the asset file tree, finding interesting files
    std::queue<tinydir_dir> dirs;
    dirs.push(dir);
    while (!dirs.empty())
    {
        tinydir_dir thisDir = dirs.front();
        dirs.pop();

        while (thisDir.has_next)
        {
            tinydir_file file;
            if (tinydir_readfile(&thisDir, &file) == -1)
            {
                LOG(ERROR) << "Couldn't read: " << thisDir.path;
                tinydir_next(&thisDir);
                continue;
            }

            try
            {
                fs::path filePath(file.path);

                // Ignore . and ..
                // Otherwise we walk forever.  Do this before absolute path!
                if (filePath.string().find("\\.") != std::string::npos ||
                    filePath.string().find("..") != std::string::npos)
                {
                    //LOG(INFO) << "Skipping: " << filePath.string();
                    tinydir_next(&thisDir);
                    continue;
                }

                // Keep paths nice and absolute/canonical
                filePath = fs::canonical(fs::absolute(filePath));
                if (checkedPaths.find(filePath) != checkedPaths.end())
                {
                    LOG(INFO) << "Already checked: " << filePath.string();
                    tinydir_next(&thisDir);
                    continue;
                }
                checkedPaths.insert(filePath);

                //LOG(INFO) << "Looking at: " << filePath.string();

                if (fs::is_directory(filePath))
                {
                    tinydir_dir subDir;
                    if (tinydir_open(&subDir, filePath.string().c_str()) != -1)
                    {
                        fs::path newPath(subDir.path);
                        newPath = fs::canonical(fs::absolute(newPath));
                        dirs.push(subDir);
                    }
                }
                else
                {
                    // Only look at non-metas and their child metas
                    if (filePath.extension().string() != ".meta")
                    {
                        AssetPath assetPath;
                        assetPath.sourcePath = filePath;

                        auto metaPath = filePath;
                        metaPath.replace_extension(".meta");
                        if (fs::exists(metaPath))
                        {
                            assetPath.sourceMetaPath = metaPath;
                        }
                        //LOG(INFO) << "Found: " << assetPath.sourcePath;

                        m_assets.push_back(assetPath);
                    }
                }
            }
            catch (fs::filesystem_error& err)
            {
                LOG(ERROR) << err.what();
            }

            tinydir_next(&thisDir);
        }
    }
   
    try
    {
        auto targetRecordFile = GetRecordFilePath();
        if (fs::exists(targetRecordFile))
        {
            json recordJson = json::parse(FileUtils::ReadFile(targetRecordFile));
            for (auto& record : recordJson["artifacts"])
            {
                auto spArtifact = std::make_shared<BuildArtifact>();
                *spArtifact = BuildArtifact(record);
                m_buildRecords[spArtifact->sourceFile.string()] = spArtifact;
            }
        }
    }
    catch (std::exception& ex)
    {
        UIManager::Instance().AddMessage(MessageType::Error | MessageType::System, ex.what());// "Couldn't load build record!");
    }
}

fs::path AssetBuilder::GetRecordFilePath() const
{
    auto targetBasePath = MAssetBuilderSettings::Instance().GetOutputPath();
    return targetBasePath / "build_record.json";
}

bool AssetBuilder::CompileAssets()
{
    auto basePath = MAssetBuilderSettings::Instance().GetInputPath();
    auto targetBasePath = MAssetBuilderSettings::Instance().GetOutputPath();

    auto targetRecordFile = GetRecordFilePath();

    if (MAssetBuilderSettings::Instance().GetVerbose())
    {
        LOG(INFO) << "Source Dir: " << basePath.string();
        LOG(INFO) << "Target Dir: " << targetBasePath.string();
    }

#if PROJECT_DEVICE_DX12
    auto spBuilder = std::make_shared<DXCompiler>();
    m_builders[".mhlsl"] = spBuilder;
#endif

    bool allOK = true;
    json buildRecord;
    for (auto& asset : m_assets)
    {
        try
        {
            auto relativePath = FileUtils::RelativeTo(basePath, asset.sourcePath);
            auto targetPath = (targetBasePath / relativePath).parent_path();

            BuildArtifact artifact;
            artifact.sourceFileTime = fs::last_write_time(asset.sourcePath);
            artifact.sourceFile = asset.sourcePath;
            artifact.sourceFileMeta = asset.sourceMetaPath;
            artifact.outputDir = targetPath;
            artifact.success = false;

            auto itrFound = m_buildRecords.find(artifact.sourceFile.string());
            if (itrFound != m_buildRecords.end())
            {
                if (itrFound->second->success == true)
                {
                    auto t1 = std::chrono::system_clock::to_time_t(itrFound->second->sourceFileTime);
                    auto t2 = std::chrono::system_clock::to_time_t(artifact.sourceFileTime);
                    if (t1 == t2)
                    {
                        // Use the old record - it hasn't changed
                        buildRecord["artifacts"].push_back(*itrFound->second);
                        if (!itrFound->second->success)
                        {
                            allOK = false;
                        }
                        continue;
                    }
                }
            }

            auto itrBuilder = m_builders.find(asset.sourcePath.extension().string());
            if (itrBuilder == m_builders.end())
            {
                Build(artifact);
            }
            else
            {
                itrBuilder->second->Build(artifact);
            }
            if (!artifact.success)
            {
                allOK = false;
            }
            buildRecord["artifacts"].push_back(artifact);
        }
        catch (fs::filesystem_error& err)
        {
            LOG(ERROR) << err.what();
        }
    }
    std::string recordString = buildRecord.dump();
    FileUtils::WriteFile(targetRecordFile, recordString.c_str(), recordString.size());

    return allOK;
}

void AssetBuilder::Build(BuildArtifact& artifact)
{

    auto outputName = artifact.outputDir / artifact.sourceFile.filename();
    artifact.outputs.push_back(outputName);

    LOG(INFO) << "Copy To: " << outputName.string();
    
    // Create missing directories
    fs::create_directories(outputName.parent_path());

    // Copy the file - default action if there is no current builder.
    if (fs::copy_file(artifact.sourceFile, outputName, fs::copy_options::overwrite_existing))
    {
        artifact.success = true;
    }
}

} // MAssetBuilder namespace