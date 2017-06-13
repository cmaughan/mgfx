#include <gtest/gtest.h>
#include "mgeo_app.h"

#include "load_assimp.h"
#include "file/media_manager.h"

using namespace MGeo;

TEST(MGeo, LoadAssimp)
{
    flatbuffers::FlatBufferBuilder builder;

    auto file = MediaManager::Instance().FindAsset("spider.obj", MediaType::Model);
    ASSERT_FALSE(file.empty());

    MGeoSettings::Instance().SetVerbose(true);
    MGeoSettings::Instance().SetGenerateTangents(true);

    ASSERT_TRUE(MGeo::LoadAssimp(file, builder));
    ASSERT_TRUE(builder.GetSize() != 0);
}
