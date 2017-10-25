#include "mcommon.h"

#if TARGET_PC
#include <windows.h>
#include <shlobj.h>
#endif

#include "fileutils.h"
#include <tinydir/tinydir.h>

#include <queue>
#include <set>

namespace FileUtils
{

std::string ReadFile(const fs::path& fileName)
{
    std::ifstream in(fileName, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(size_t(in.tellg()));
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    return std::string();
}

bool WriteFile(const fs::path& fileName, const void* pData, size_t size)
{
    FILE* pFile;
    pFile = fopen(fileName.string().c_str(), "wb");
    if (!pFile)
    {
        return false;
    }
    fwrite(pData, sizeof(uint8_t), size, pFile);
    fclose(pFile);
    return true;
}

// http://stackoverflow.com/a/29221546/18942
fs::path RelativeTo(fs::path from, fs::path to)
{
    // Start at the root path and while they are the same then do nothing then when they first
    // diverge take the remainder of the two path and replace the entire from path with ".."
    // segments.
    fs::path::const_iterator fromIter = from.begin();
    fs::path::const_iterator toIter = to.begin();

    // Loop through both
    while (fromIter != from.end() && toIter != to.end() && (*toIter) == (*fromIter))
    {
        ++toIter;
        ++fromIter;
    }

    fs::path finalPath;
    while (fromIter != from.end())
    {
        finalPath = finalPath / "..";
        ++fromIter;
    }

    while (toIter != to.end())
    {
        finalPath = finalPath / *toIter;
        ++toIter;
    }

    return finalPath;
}

fs::path GetDocumentsPath()
{
#if TARGET_PC
    PWSTR path;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
    if (SUCCEEDED(hr)) 
    {
        fs::path ret = StringUtils::makeStr(path);
        CoTaskMemFree(path);
        return ret;
    }
    return fs::path();
#else
    auto pszHome = getenv("HOME");
    return fs::path(std::string(pszHome) + "/Documents");
#endif
}

std::vector<fs::path> GatherFiles(const fs::path& root)
{
    std::vector<fs::path> ret;

    tinydir_dir dir;
    if (tinydir_open(&dir, root.string().c_str()) == -1)
    {
        LOG(ERROR) << "Gather Files, Start Path Invalid: " << root.string();
        return ret;
    }

    std::set<fs::path> checkedPaths;
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
                    ret.push_back(filePath);
                }
            }
            catch (fs::filesystem_error& err)
            {
                LOG(ERROR) << err.what();
            }

            tinydir_next(&thisDir);
        }
    }
    return ret;
}

} // FileUtils