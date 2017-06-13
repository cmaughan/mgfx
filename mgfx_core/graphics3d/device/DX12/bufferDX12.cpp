#include "mgfx_core.h"
#include "deviceDX12.h"
#include "bufferDX12.h"

namespace Mgfx
{

BufferDX12::BufferDX12(DeviceDX12* pDevice, int size, uint32_t flags)
    : m_pDevice(pDevice),
    m_size(0),
    m_flags(flags)
{
    EnsureSize(size);
}

BufferDX12::~BufferDX12()
{
}

uint32_t BufferDX12::GetByteSize() const
{
    return m_size;
}

void BufferDX12::EnsureSize(uint32_t size)
{
    if (m_size >= size)
    {
        return;
    }
    
    if (m_flags & DeviceBufferFlags::UseUploadBuffer)
    {
        assert(!"Upload buffer type not yet implemented");
    }

    m_buffer.Destroy();

    m_buffer.Create(L"BufferDX12", size, sizeof(uint8_t));
    m_pBufferData = (uint8_t*)m_buffer.Map();
    m_size = size;
    m_startOffset = 0;
    m_currentOffset = 0;
}

void BufferDX12::Upload()
{
    if (!(m_flags & DeviceBufferFlags::UseUploadBuffer))
    {
        return;
    }

    // Do upload... Currently we only use direct mapped buffers
}

void* BufferDX12::Map(uint32_t num, uint32_t typeSize, uint32_t& offset)
{
    uint32_t byteSize = num * typeSize;

    if (m_pBufferData == nullptr)
    {
        return nullptr;
    }

    if ((byteSize + m_currentOffset) >= m_size)
    {
        // Jump back to the beginning
        m_startOffset = 0;
        m_currentOffset = 0;
    }

    // Return the last offset we were up to
    m_startOffset = m_currentOffset;
    offset = m_startOffset / typeSize;
    m_currentOffset = m_currentOffset + byteSize;
    return (void*)(m_pBufferData + m_startOffset);
}

void BufferDX12::UnMap()
{
   // Permanently mapped
   Upload();
}

} // namespace Mgfx
