

LIST(APPEND MGFX_APP_SOURCES
    mgfx/app/mgfx.exe.manifest
    mgfx/app/main.cpp
    mgfx/app/Sponza.cpp
    mgfx/app/Sponza.h
    mgfx/app/Asteroids.cpp
    mgfx/app/Asteroids.h
    mgfx/app/GeometryTest.cpp
    mgfx/app/GeometryTest.h
    mgfx/app/GameOfLife.cpp
    mgfx/app/GameOfLife.h
    mgfx/app/Mazes.cpp
    mgfx/app/Mazes.h
    mgfx/app/RayTracer.cpp
    mgfx/app/RayTracer.h
    mgfx/app/MgfxRender.cpp
    mgfx/app/MgfxRender.h
    mgfx/app/mgfx_app.h
    mgfx/app/mgfx_settings.cpp
    mgfx/app/mgfx_settings.h
    mgfx/list.cmake
)

LIST(APPEND TEST_SOURCES
    mgfx/app/mgfx_settings.cpp
    mgfx/app/mgfx_settings.h
)