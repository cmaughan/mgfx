#pragma once

namespace MAssetBuilder
{

class MAssetBuilderSettings
{
public:
    static MAssetBuilderSettings& Instance();

    bool GetVerbose() const { return m_verbose; }
    void SetVerbose(bool verbose) { m_verbose = verbose; }

    const fs::path& GetInputPath() const { return m_inputPath; }
    const fs::path& GetOutputPath() const { return m_outputPath; }
    void SetInputPath(const fs::path& path);
    void SetOutputPath(const fs::path& path);

private:
    bool m_verbose = false;
    fs::path m_inputPath;
    fs::path m_outputPath;
};

} // namespace MAssetBuilder