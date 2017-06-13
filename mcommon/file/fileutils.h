#pragma once

#if TARGET_MAC
#undef PROJECT_CPP_FILESYSTEM
#endif

#ifdef PROJECT_CPP_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;
#else
#include <file/mfilesystem.h>
namespace fs = mfilesystem;
#endif

namespace FileUtils
{
std::string ReadFile(const fs::path& fileName);
bool WriteFile(const fs::path& fileName, const void* pData, size_t size);
fs::path RelativeTo(fs::path from, fs::path to);
}
