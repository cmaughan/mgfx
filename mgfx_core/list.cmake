
LIST(APPEND MGFX_SOURCES
   mgfx_core/graphics3d/device/IDevice.h 
   mgfx_core/graphics3d/camera/camera.h
   mgfx_core/graphics3d/camera/camera.cpp
   mgfx_core/graphics3d/geometry/mesh.cpp
   mgfx_core/graphics3d/geometry/mesh.h
   mgfx_core/graphics3d/ui/camera_manipulator.h
   mgfx_core/graphics3d/ui/camera_manipulator.cpp
   mgfx_core/graphics3d/scene/scene.h
   mgfx_core/graphics3d/scene/scene.cpp
   mgfx_core
   mgfx_core/mgfx_core.h
   mgfx_core/graphics2d/ui/imgui_sdl_common.cpp
   mgfx_core/graphics2d/ui/imgui_sdl_common.h
   mgfx_core/graphics2d/ui/windowmanager.cpp
   mgfx_core/graphics2d/ui/windowmanager.h
   mgfx_core/graphics2d/ui/window.cpp
   mgfx_core/graphics2d/ui/window.h
   mgfx_core/list.cmake
)

IF (PROJECT_DEVICE_GL)

LIST(APPEND MGFX_SOURCES
   mgfx_core/graphics3d/device/GL/deviceGL.cpp
   mgfx_core/graphics3d/device/GL/deviceGL.h
   mgfx_core/graphics3d/device/GL/geometryGL.cpp
   mgfx_core/graphics3d/device/GL/geometryGL.h
   mgfx_core/graphics3d/device/GL/bufferGL.cpp
   mgfx_core/graphics3d/device/GL/bufferGL.h
   mgfx_core/graphics3d/device/GL/glcorearb.h
   mgfx_core/graphics3d/device/GL/shader.cpp
   mgfx_core/graphics3d/device/GL/shader.h
   mgfx_core/graphics3d/device/GL/imguisdl_gl3.cpp
   mgfx_core/graphics3d/device/GL/imguisdl_gl3.h
)

ENDIF()

IF (PROJECT_DEVICE_DX12)

LIST(APPEND MGFX_SOURCES
   mgfx_core/graphics3d/device/DX12/deviceDX12.cpp
   mgfx_core/graphics3d/device/DX12/deviceDX12.h
   mgfx_core/graphics3d/device/DX12/imguisdl_dx12.cpp
   mgfx_core/graphics3d/device/DX12/imguisdl_dx12.h
   mgfx_core/graphics3d/device/DX12/geometryDX12.cpp
   mgfx_core/graphics3d/device/DX12/geometryDX12.h
   mgfx_core/graphics3d/device/DX12/meshDX12.cpp
   mgfx_core/graphics3d/device/DX12/meshDX12.h
   mgfx_core/graphics3d/device/DX12/bufferDX12.cpp
   mgfx_core/graphics3d/device/DX12/bufferDX12.h
   mgfx_core/graphics3d/device/DX12/miniengine.h
   mgfx_core/graphics3d/device/DX12/miniengine/DDSTextureLoader.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/TextureManager.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/CommandContext.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/BuddyAllocator.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/BufferManager.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/Color.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/ColorBuffer.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/CommandAllocatorPool.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/CommandAllocatorPool.h
   mgfx_core/graphics3d/device/DX12/miniengine/CommandContext.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/CommandContext.h
   mgfx_core/graphics3d/device/DX12/miniengine/CommandListManager.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/CommandListManager.h
   mgfx_core/graphics3d/device/DX12/miniengine/CommandSignature.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/CommandSignature.h
   mgfx_core/graphics3d/device/DX12/miniengine/DepthBuffer.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/DescriptorHeap.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/DynamicDescriptorHeap.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/DynamicUploadBuffer.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/FileUtility.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/GpuBuffer.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/GraphicsCore.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/GpuTimeManager.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/GraphicsCommon.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/LinearAllocator.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/SystemTime.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/PipelineState.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/PixelBuffer.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/RootSignature.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/SamplerManager.cpp
   mgfx_core/graphics3d/device/DX12/miniengine/Utility.cpp 
   mgfx_core/graphics3d/device/DX12/miniengine/EngineProfiling.cpp 
)

LIST(APPEND PLATFORM_LINKLIBS 
    d3d12
    dxgi
)
ENDIF()

if (PROJECT_DEVICE_NULL)

LIST(APPEND MGFX_SOURCES
    mgfx_core/graphics3d/device/Null/deviceNull.cpp
    mgfx_core/graphics3d/device/Null/deviceNull.h
)

endif()

LIST(APPEND MGFX_INCLUDE
    mgfx_core
    mgfx_core/graphics3d
    mgfx_core/graphics3d/device
    mgfx_core/graphics2d
)
