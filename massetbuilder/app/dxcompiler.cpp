#include "massetbuilder_app.h"
#include "dxcompiler.h"
#include "d3dcompiler.h"
#include "ui/ui_manager.h"
#include "file/fileutils.h"
#include "file/media_manager.h"
#include <locale>
#include <codecvt>
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;

namespace MAssetBuilder
{

HRESULT DXCompiler::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
    auto inc = MediaManager::Instance().LoadAsset(pFileName, MediaType::Shader, &currentRootPath);
    if (inc.empty())
    {
        LOG(ERROR) << "#include: " << pFileName << " failed!";
        return E_FAIL;
    }

    auto pMem = new uint8_t[inc.size()];
    memcpy(pMem, inc.c_str(), inc.size());
    *ppData = pMem;
    *pBytes = UINT(inc.size());
    return S_OK;
}

HRESULT DXCompiler::Close(LPCVOID pData)
{
    delete ((uint8_t*)pData);
    return S_OK;
}

void DXCompiler::Build(BuildArtifact& artifact)
{
    ComPtr<ID3DBlob> pShader;
    ComPtr<ID3DBlob> pErrors;

    LOG(INFO) << "Compiling (DX): " << artifact.sourceFile.string() << "...";

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    std::map<std::string, std::string> fileNameToShaderType;
    fileNameToShaderType["PS"] = "ps_5_0";
    fileNameToShaderType["VS"] = "vs_5_0";
    fileNameToShaderType["GS"] = "gs_5_0";
    fileNameToShaderType["CS"] = "cs_5_0";

    currentRootPath = artifact.sourceFile.parent_path();

    auto fileStem = artifact.sourceFile.stem();
    auto outputPath = artifact.outputDir / fileStem;
    outputPath.replace_extension(".cso");

    artifact.outputs.push_back(outputPath);

    // Try to find the shader type
    std::string entryPoint = fileStem.string();
    std::string shaderType;
    for (auto& search : fileNameToShaderType)
    {
        auto pos = entryPoint.find(search.first);
        if (pos != std::string::npos)
        {
            shaderType = search.second;
            break;
        }
    }

    if (entryPoint.empty())
    {
        UIManager::Instance().AddMessage(MessageType::Error, "Failed to find shader entry point", artifact.sourceFile);
        return;
    }

    if (shaderType.empty())
    {
        UIManager::Instance().AddMessage(MessageType::Error, "Failed to find shader type", artifact.sourceFile);
        return;
    }
    
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(artifact.sourceFile.string());
    if (!FAILED(D3DCompileFromFile(wide.c_str(), nullptr, this, entryPoint.c_str(), shaderType.c_str(), compileFlags, 0, &pShader, &pErrors)))
    {
        artifact.success = true;
        fs::create_directories(outputPath.parent_path());
        FileUtils::WriteFile(outputPath, pShader->GetBufferPointer(), pShader->GetBufferSize());
    }
    else
    {
        // We need to filter:
        // <filepath>(<linenum>,<column>-?<column>): message
        std::string output = std::string((const char*)pErrors->GetBufferPointer());

        // Try to parse the DX error string into file, line, column and message
        // Exception should catch silly mistakes.
        auto errors = StringUtils::Split(output, "\n");
        for (auto error : errors)
        {
            std::string fileName = artifact.sourceFile.string();
            std::string message;
            ColumnRange range{ -1, -1 };
            int32_t line = -1;
       
            StringUtils::Trim(error);
            
            try
            {
                auto bracketPos = error.find_first_of('(');
                if (bracketPos != std::string::npos)
                {
                    auto lastBracket = error.find("):", bracketPos);
                    if (lastBracket)
                    {
                        fileName = StringUtils::Trim(error.substr(0, bracketPos));
                        message = StringUtils::Trim(error.substr(lastBracket + 2, error.size() - lastBracket + 2));
                        std::string numbers = StringUtils::Trim(error.substr(bracketPos, lastBracket - bracketPos), "( )");
                        auto numVec = StringUtils::Split(numbers, ",");
                        if (!numVec.empty())
                        {
                            line = std::stoi(numVec[0]);
                        }
                        if (numVec.size() > 1)
                        {
                            auto columnVec = StringUtils::Split(numVec[1], "-");
                            if (!columnVec.empty())
                            {
                                range.start = std::stoi(columnVec[0]);
                                if (columnVec.size() > 1)
                                {
                                    range.end = std::stoi(columnVec[1]);
                                }
                            }
                        }
                    }
                }
                else
                {
                    message = error;
                }
            }
            catch (...)
            {
                // Ignore parse failures:
                UIManager::Instance().AddMessage(MessageType::Error, "Failed to parse error string: " + output);
            }

            UIManager::Instance().AddMessage(MessageType::Error, message, fileName, line, range);
        }
    }
}

} // MAssetBuilder namespace
