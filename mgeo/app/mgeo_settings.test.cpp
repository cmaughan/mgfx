#include "mgeo_app.h"
#include <gtest/gtest.h>
#include "mgeo/app/mgeo_settings.h"

TEST(App, MGeoSettings)
{
    MGeoSettings::Instance().SetVerbose(true);
    auto verbose = MGeoSettings::Instance().GetVerbose();
    ASSERT_EQ(verbose, true);

    MGeoSettings::Instance().SetGenerateTangents(true);
    auto tangents = MGeoSettings::Instance().GetGenerateTangents();
    ASSERT_EQ(tangents, true);
}