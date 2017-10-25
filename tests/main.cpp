#include <gtest/gtest.h>
#include "m3rdparty.h"
#include "mcommon.h"
#include "file/media_manager.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[])
{
    // Setup Logger
    fs::path basePath = SDL_GetBasePath();
    el::Configurations conf((basePath / "logger.conf").string().c_str());
    el::Loggers::reconfigureAllLoggers(conf);

    // Setup Asset path
    basePath = basePath / "assets";
    basePath = fs::absolute(basePath);
    if (fs::exists(basePath))
    {
        basePath = fs::canonical(basePath);
    }
    MediaManager::Instance().SetAssetPath(basePath);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

