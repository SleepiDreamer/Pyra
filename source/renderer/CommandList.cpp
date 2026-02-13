#include "CommandList.h"
#include "Fence.h"
#include "SwapChain.h"
#include "HelpersDX.h"

#include <cassert>

using namespace Microsoft::WRL;

CommandList::CommandList(const ComPtr<ID3D12Device10>& device, const D3D12_COMMAND_LIST_TYPE type) : m_type(type)
{
    for (uint32_t i = 0; i < SwapChain::m_numBuffering; ++i)
    {
        ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_allocators[i])));
    }

    ThrowIfFailed(device->CreateCommandList(0, type, m_allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    const wchar_t* name = nullptr;
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:  name = L"Graphics CmdList"; break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE: name = L"Compute CmdList";  break;
    case D3D12_COMMAND_LIST_TYPE_COPY:    name = L"Copy CmdList";     break;
    default:
        throw std::runtime_error("Unsupported command list type!");
    }
    ThrowIfFailed(m_commandList->SetName(name));

    ThrowIfFailed(m_commandList->Close());
    m_cmdListClosed = true;
}

void CommandList::Reset(const uint32_t frameIndex, const Fence& fence)
{
    assert(frameIndex < SwapChain::m_numBuffering);

    const uint64_t requiredValue = m_allocatorFenceValues[frameIndex];
    if (requiredValue > 0)
    {
        fence.WaitFor(requiredValue);
    }

    ThrowIfFailed(m_allocators[frameIndex]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_allocators[frameIndex].Get(), nullptr));
    m_cmdListClosed = false;
}

void CommandList::Close()
{
    if (m_cmdListClosed)
    {
        return;
    }
    ThrowIfFailed(m_commandList->Close());
    m_cmdListClosed = true;
}

void CommandList::StartMarker(const std::string& name) const
{
    m_commandList->BeginEvent(1u, name.c_str(), static_cast<UINT>(name.size() + 1));
}

void CommandList::EndMarker() const
{
	m_commandList->EndEvent();
}