#include "mgfx_app.h"
#include "mgfx_settings.h"

#include "MgfxRender.h"

MgfxSettings& MgfxSettings::Instance()
{
    static MgfxSettings settings;
    return settings;
}

MgfxRender* MgfxSettings::GetCurrentRenderer() const
{
    return m_pCurrentRenderer;
}

void MgfxSettings::SetCurrentRenderer(MgfxRender* pRender)
{
    m_pCurrentRenderer = pRender;
}

void MgfxSettings::AddRenderer(std::shared_ptr<MgfxRender> spRender)
{
    m_renderers.push_back(spRender);
    if (m_pCurrentRenderer == nullptr)
    {
        m_pCurrentRenderer = spRender.get();
    }
}

void MgfxSettings::ClearRenderers()
{
    m_renderers.clear();
    m_pCurrentRenderer = nullptr;
}

const std::vector<std::shared_ptr<MgfxRender>>& MgfxSettings::GetRenderers() const
{
    return m_renderers;
}
