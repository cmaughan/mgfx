#pragma once

struct MgfxRender;
class MgfxSettings
{
public:
    enum class Device
    {
        GL,
        DX12
    };

    static MgfxSettings& Instance();

    MgfxRender* GetCurrentRenderer() const;
    void SetCurrentRenderer(MgfxRender* render);

    void SetDevice(Device device) { m_device = device; }
    Device GetDevice() const { return m_device; }

    void AddRenderer(std::shared_ptr<MgfxRender> spRender);
    const std::vector<std::shared_ptr<MgfxRender>>& GetRenderers() const;
    void ClearRenderers();

    void SetConsole(bool console) { m_bConsole = console; }
    bool GetConsole() const { return m_bConsole; }

private:
    std::vector<std::shared_ptr<MgfxRender>> m_renderers;
    MgfxRender* m_pCurrentRenderer = nullptr;
    Device m_device;
    bool m_bConsole;
};

