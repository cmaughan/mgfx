#pragma once
#include "miniengine/DynamicUploadBuffer.h"

namespace Mgfx
{

class BufferDX12 : public IDeviceBuffer
{
public:
    BufferDX12(DeviceDX12* pDevice, int size, uint32_t flags);
    ~BufferDX12();

    // Return a pointer to a buffer, with num elements of size typesize free, and the offset 
    // where the buffer was mapped (for later rendering)
    virtual void* Map(uint32_t num, uint32_t typeSize, uint32_t& offset) override;
    virtual void UnMap() override;
    virtual void EnsureSize(uint32_t size) override;
    virtual void Upload() override;
    virtual uint32_t GetByteSize() const override;
    virtual void Bind() const override { /* nothing */ };
    virtual void UnBind() const override { /* nothing */ };

public:
    const DynamicUploadBuffer& GetBuffer() const { return m_buffer; }

private:
    DynamicUploadBuffer m_buffer;
    DeviceDX12* m_pDevice = nullptr;
    uint32_t m_currentOffset = 0;
    uint32_t m_startOffset = 0;
    uint8_t* m_pBufferData = nullptr;
    uint32_t m_size = 0;
    uint32_t m_flags = 0;
};

} // namespace Mgfx
