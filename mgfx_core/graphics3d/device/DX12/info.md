# Device Rendering

Initialization

- (Debug) Create Debug Layers
- DXGI Factory
    - Adapter HW (find one)
        - Create Device (Feature Level 11)

- Create CommandQueue

- Create Swapchain
    - Window Association
    - Current Backbuffer Index

- Create DescriptorHeap
    - One for RenderTargetViews
    - One for Shader Views
    - ::GetDescriptorHandleIncrementSize (Size of each Descriptor in the heap)
    - RTVHandle = ::GetCPUDescriptorHandleForHeapStart
        - Each RenderTarget Frame:
            - ::CreateRenderTargetView (RTVHandle)
            - Offset handle by descriptor size

- Create Command Allocator

- Create Root Signature (Sampler?)
    - FEATURE_DATA
    - DESCRIPTOR_RANGE SRV STATIC
    - ROOT_PARAMETER DescriptorTable Shader/Pixel Visibility
    - ROOT_SIGNATURE_DESC ^ Parameter, Sampler
    - Serialize the root signature
    - Create it.

- Create Command List
    - Close it (open after creation)

- Create Fence (default 0?)
    - Init fence value to 1
    - Create Event - Windows Thread stuff
    - ::WaitForPreviousFrame (waiting for resources, not needed yet)

- Swap
    - Cmd: Resource Barrier - RTV Read to Present
    - Cmd: Close
    - Queue: Send Cmd
    - Swap
    - ::WaitForPreviousFrame

# Begin3D
    - Reset Command Allocator
    - Reset Command List
    - Cmd: SetRootSignature
    - Cmd: SetHeaps
    - Cmd: SetViewport, Scissors
    - Cmd: Barrier: Present->Render
    - Cmd: SetRenderTargets based on Heap:DescriptorStart + frameCount
    - Cmd: Clear RenderTargetView

# End3D
    - Cmd: Close
    - Queue: Send Cmd

# Begin2D
    - if not created:
        - Create Root Signature
            - Sampler for font
            - PSO for Triangles, Rasterizer, etc.
            - Create Shaders
            - Create Font - own heap?
            - Updload
            - Barrier upload->pixel resource
            - Create command list
                - Temporary for upload, setup
# End2D
    - Submit to ImGui
    - Add to Cmd:

(Methods)
- WaitForPreviousFrame
    - Cmd: Signal Fence (fenceValue)
    - Fence: GetCompletedValue < fence
    - Fence: Wait for Completion fenceValue == Fence
    - Get new Frame Index

