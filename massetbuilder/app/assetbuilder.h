#pragma once

#include "m3rdparty/json/src/json.hpp"
#include <chrono>

namespace MAssetBuilder
{

struct BuildArtifact
{
    // Inputs
    fs::path outputDir;                 // Target file directory, if in the same relative path
    fs::path sourceFile;                // The source file used in the build
    fs::path sourceFileMeta;            // The source meta file used in the build
    fs::file_time_type sourceFileTime;  // The time the source file was modified when built

    // Outputs
    bool success;                       // If the compile succeeded
    std::vector<fs::path> outputs;      // Outputs of the build step
};
struct IBuilder
{
    virtual void Build(BuildArtifact& artifact) = 0;
};

class AssetBuilder : public IBuilder
{
public:
    struct AssetPath
    {
        fs::path sourcePath;
        fs::path sourceMetaPath;
    };

    AssetBuilder();

    void GatherAssets();
    bool CompileAssets();

    // Default builder
    virtual void Build(BuildArtifact& artifact) override;

private:
    fs::path GetRecordFilePath() const;

private:
    std::map<std::string, std::shared_ptr<IBuilder>> m_builders;
    std::vector<AssetPath> m_assets;
    std::vector<BuildArtifact> m_outputs;
    std::map<std::string, std::shared_ptr<BuildArtifact>> m_buildRecords;
};

} // namespace MAssetBuilder 
