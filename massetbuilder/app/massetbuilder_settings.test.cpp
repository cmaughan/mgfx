#include "massetbuilder_app.h"
#include <gtest/gtest.h>
#include "mgeo/app/mgeo_settings.h"

using namespace MAssetBuilder;

TEST(App, MAssetBuilderSettings)
{
    MAssetBuilderSettings::Instance().SetVerbose(true);
    auto verbose = MAssetBuilderSettings::Instance().GetVerbose();
    ASSERT_EQ(verbose, true);

    MAssetBuilderSettings::Instance().SetInputPath(fs::path("e:/foo/bar"));
    MAssetBuilderSettings::Instance().SetOutputPath(fs::path("e:/foo/bar"));
}