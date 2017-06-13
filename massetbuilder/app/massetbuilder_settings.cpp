#include "massetbuilder_app.h"

namespace MAssetBuilder
{

MAssetBuilderSettings& MAssetBuilderSettings::Instance()
{
    static MAssetBuilderSettings settings;
    return settings;
}

void MAssetBuilderSettings::SetInputPath(const fs::path& path)
{
    try
    {
        m_inputPath = fs::canonical(fs::absolute(path));
    }
    catch (fs::filesystem_error& err)
    {
        LOG(ERROR) << err.what();
    }
}

void MAssetBuilderSettings::SetOutputPath(const fs::path& path)
{
    try
    {
        m_outputPath = fs::absolute(path);
        fs::create_directories(m_outputPath);
        m_outputPath = fs::canonical(m_outputPath);
        LOG(INFO) << "Asset Output Path: " << m_outputPath.string();
    }
    catch (fs::filesystem_error& err)
    {
        LOG(ERROR) << err.what();
    }
}

} // namespace MAssetBuilder;