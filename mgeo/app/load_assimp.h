#pragma once

#include "schema/model_generated.h"

namespace MGeo
{

bool LoadAssimp(fs::path filePath, flatbuffers::FlatBufferBuilder& builder);

} // namespace MGeo
