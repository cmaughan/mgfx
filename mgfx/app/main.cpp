#include "mgfx_app.h"

#if PROJECT_DEVICE_GL
#include <GL/deviceGL.h>
#endif

#if PROJECT_DEVICE_DX12
#include <DX12/deviceDX12.h>
#endif

#if PROJECT_DEVICE_NULL
#include <Null/deviceNull.h>
#endif

#include "mgfx_settings.h"

#include "ui/windowmanager.h"
#include "ui/window.h"

#include "file/media_manager.h"

#include "tclap/CmdLine.h"

#include "Asteroids.h"
#include "GeometryTest.h"
#include "Sponza.h"
#include "GameOfLife.h"
#include "Mazes.h"
#include "RayTracer.h"

INITIALIZE_EASYLOGGINGPP

using namespace Mgfx;

std::map<Window*, MgfxRender*> WindowRenderers;
std::map<Window*, MgfxRender*> PendingWindowRenderers;

namespace
{
struct Properties
{
    bool SyncRefresh = true;
};

Properties properties;
}

void SwitchRenderer(Window* pWindow, MgfxRender* pRender)
{
    pWindow->GetDevice()->Flush();

    auto itrCurrent = WindowRenderers.find(pWindow);
    if (itrCurrent != WindowRenderers.end())
    {
        // Nothing to do
        if (itrCurrent->second == pRender)
        {
            return;
        }

        itrCurrent->second->RemoveFromWindow(pWindow);
        itrCurrent->second->CleanUp();
    }

    pWindow->GetDevice()->SetClear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.0f);
    pRender->Init();
    pRender->AddToWindow(pWindow);
    WindowRenderers[pWindow] = pRender;
}

// This is where we show our app GUI.
void ShowGUI(Window* pWindow)
{
    // Finally, the GUI
    pWindow->GetDevice()->BeginGUI();

    static bool show_test_window = false;

    int selected = 0;
    const auto& renderers = MgfxSettings::Instance().GetRenderers();
    auto pCurrentRenderer = MgfxSettings::Instance().GetCurrentRenderer();
    std::vector<const char*> strRenders;
    for (int i = 0; i < int(renderers.size()); i++)
    {
        strRenders.push_back(renderers[i]->Name());
        if (renderers[i].get() == pCurrentRenderer)
        {
            selected = i;
        }
    }

    if (!strRenders.empty())
    {
        if (ImGui::Combo("Renderer", &selected, &strRenders[0], int(strRenders.size())))
        {
            PendingWindowRenderers[pWindow] = renderers[selected].get();
        }
    }

    if (ImGui::CollapsingHeader("Settings"))
    {
        // draw with the current one before switching
        pCurrentRenderer->DrawGUI(pWindow);
    }


    int drawX, drawY;
    SDL_GL_GetDrawableSize(pWindow->GetDevice()->GetSDLWindow(), &drawX, &drawY);

    if (ImGui::CollapsingHeader("Description"))
    {
        ImGui::TextWrapped("%s", renderers[selected]->Description());
    }

    ImGui::Separator();
    ImGui::Text("%d x %d Window Size\n%d x %d Drawable",
        int(ImGui::GetIO().DisplaySize.x),
        int(ImGui::GetIO().DisplaySize.y),
        drawX,
        drawY);

    if (ImGui::Checkbox("Lock to Refresh", &properties.SyncRefresh))
    {
        pWindow->GetDevice()->SetDeviceFlags(properties.SyncRefresh ? DeviceFlags::SyncToRefresh : 0);
    }
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    // For testing the GUI features
    if (ImGui::Button("GUI Features"))
    {
        show_test_window ^= 1;
    }
    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&show_test_window);
    }

    pWindow->GetDevice()->EndGUI();
}

bool ReadCommandLine(int argc, char** argv, int& exitCode)
{

    try
    {
        TCLAP::CmdLine cmd(APPLICATION_NAME, ' ', APPLICATION_VERSION);
        TCLAP::SwitchArg gl("", "gl", "Enable OpenGL", cmd, false);
        TCLAP::SwitchArg d3d("", "d3d", "Enable DX12", cmd, false);
        TCLAP::SwitchArg console("c", "console", "Enable Console", cmd, false);

        cmd.setExceptionHandling(false);
        cmd.ignoreUnmatched(false);

        if (argc != 0)
        {
            cmd.parse(argc, argv);

            MgfxSettings::Instance().SetConsole(console.getValue());
#if TARGET_PC
            // Show the console if the user supplied args
            // On a Win32 app, this isn't available by default
            if (console.getValue())
            {
                AllocConsole();
                freopen("CONIN$", "r", stdin);
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
            }
#endif

#ifdef PROJECT_DEVICE_DX12
            if (d3d.getValue())
            {
                MgfxSettings::Instance().SetDevice(MgfxSettings::Device::DX12);
            }
            else
#endif
            {
                MgfxSettings::Instance().SetDevice(MgfxSettings::Device::GL);
            }
        }
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        // Report argument exceptions to the message box
        std::ostringstream strError;
        strError << e.argId() << " : " << e.error();
        UIManager::Instance().AddMessage(MessageType::Error | MessageType::System, strError.str());
        exitCode = 1;
        return false;
    }
    catch (TCLAP::ExitException& e)
    {
        // Allow PC app to continue, and ignore exit due to help/version
        // This avoids the danger that the user passed --help on the command line and wondered why the app just exited
        exitCode = e.getExitStatus();
        return true;
    }
    return true;
}

template <class DeviceT>
void CreateDevice(std::vector<std::shared_ptr<IDevice>>& devices)
{
    auto pDevice = std::static_pointer_cast<IDevice>(std::make_shared<DeviceT>());
    if (!pDevice->Init())
    {
        UIManager::Instance().AddMessage(MessageType::Error | MessageType::System,
            std::string("Couldn't create device: ") + std::string(pDevice->GetName()));
    }
    else
    {
        pDevice->SetDeviceFlags(properties.SyncRefresh ? DeviceFlags::SyncToRefresh : 0);
    }
    devices.push_back(pDevice);
}


int main(int argc, char** argv)
{
    fs::path basePath = SDL_GetBasePath();
    el::Configurations conf((basePath / "logger.conf").string().c_str());
    el::Loggers::reconfigureAllLoggers(conf);

    basePath = basePath / "assets";
    basePath = fs::absolute(basePath);
    if (fs::exists(basePath))
    {
        basePath = fs::canonical(basePath);
    }
    MediaManager::Instance().SetAssetPath(basePath);

    int exitCode = 0;
    if (!ReadCommandLine(argc, argv, exitCode))
    {
        return exitCode;
    }

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        LOG(ERROR) << SDL_GetError();
        return -1;
    }


    MgfxSettings::Instance().AddRenderer(std::make_shared<Asteroids>());
    MgfxSettings::Instance().AddRenderer(std::make_shared<Sponza>());
    MgfxSettings::Instance().AddRenderer(std::make_shared<Mazes>());
    MgfxSettings::Instance().AddRenderer(std::make_shared<GameOfLife>());
    MgfxSettings::Instance().AddRenderer(std::make_shared<RayTracer>());
    
    //MgfxSettings::Instance().AddRenderer(std::make_shared<GeometryTest>());

    std::vector<std::shared_ptr<IDevice>> vecDevices;

#if PROJECT_DEVICE_GL
    if (MgfxSettings::Instance().GetDevice() == MgfxSettings::Device::GL)
    {
        CreateDevice<DeviceGL>(vecDevices);
    }
#endif

#if PROJECT_DEVICE_NULL
    //CreateDevice<DeviceNull>(vecDevices);
#endif

#if PROJECT_DEVICE_DX12
    if (MgfxSettings::Instance().GetDevice() == MgfxSettings::Device::DX12)
    {
        CreateDevice<DeviceDX12>(vecDevices);
    }
#endif

    auto pCurrentRender = MgfxSettings::Instance().GetCurrentRenderer();
    for (auto& pDevice : vecDevices)
    {
        auto pWindow = WindowManager::Instance().AddWindow(pDevice->GetSDLWindow(), pDevice);
        SwitchRenderer(pWindow, pCurrentRender);
    }

    Timer frameTimer;

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            // Get the SDL and the Window wrapper for this event
            auto pSDLWindow = WindowManager::Instance().GetSDLWindowFromEvent(e);
            auto pWindow = WindowManager::Instance().GetWindow(pSDLWindow);

            // Allow the window to handle any events of its own
            // (will defer to any handlers)
            if (pWindow)
            {
                pWindow->ProcessEvent(e);
            }

            if (e.type == SDL_QUIT)
            {
                done = true;
                break;
            }

            switch (e.type)
            {
            case SDL_WINDOWEVENT:
            {
                if (pWindow)
                {
                    switch (e.window.event)
                    {
                        // Window close will remove and destroy the SDL_Window
                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        if (pWindow)
                        {
                            WindowManager::Instance().RemoveWindow(pWindow);
                        }
                    }
                    break;

                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        if (WindowRenderers.find(pWindow) != WindowRenderers.end())
                        {
                            auto pRenderer = WindowRenderers[pWindow];
                            pRenderer->ResizeWindow(pWindow);
                        }
                    }
                    break;

                    break;
                    default:
                        break;
                    }
                }
            }
            break;
            default:
                continue;
            }
        }

        auto frameDelta = frameTimer.GetDelta();
        frameTimer.Restart();

        // No more events, lets do some drawing
        // Walk the list of windows currently drawing
        for (auto& windows : WindowManager::Instance().GetWindows())
        {
            auto& spWindow = windows.second;

            if (PendingWindowRenderers.find(spWindow.get()) != PendingWindowRenderers.end())
            {
                MgfxSettings::Instance().SetCurrentRenderer(PendingWindowRenderers[spWindow.get()]);
                SwitchRenderer(spWindow.get(), PendingWindowRenderers[spWindow.get()]);
                PendingWindowRenderers.erase(spWindow.get());
            }

            auto pRenderer = WindowRenderers[spWindow.get()];

            spWindow->PreRender(frameDelta);

            if (pRenderer)
            {
                if (spWindow->GetDevice()->BeginFrame())
                {
                    pRenderer->Render(spWindow.get());

                    ShowGUI(spWindow.get());

                    // Display result
                    spWindow->GetDevice()->Swap();
                }
            }
        }
    }

    if (MgfxSettings::Instance().GetCurrentRenderer())
    {
        // Walk the list of windows currently drawing
        for (auto& windows : WindowManager::Instance().GetWindows())
        {
            MgfxSettings::Instance().GetCurrentRenderer()->RemoveFromWindow(windows.second.get());
        }
        MgfxSettings::Instance().GetCurrentRenderer()->CleanUp();
    }

    MgfxSettings::Instance().ClearRenderers();

    ImGui::Shutdown();
    SDL_Quit();

    return 0;
}
