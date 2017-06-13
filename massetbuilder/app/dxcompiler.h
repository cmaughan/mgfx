#pragma once
#include "mgfx_core/graphics3d/device/DX12/deviceDX12.h"
#include "json/src/json.hpp"
#include "assetbuilder.h"

namespace MAssetBuilder
{

class DXCompiler : public IBuilder, ID3DInclude
{
public:
    virtual void Build(BuildArtifact& artifact) override;

    virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override;
    virtual HRESULT Close(LPCVOID pData) override;

    fs::path currentRootPath;
};

}