#include "CommandQueue.h"
#include "CommandList.h"
#include "Fence.h"
#include "HelpersDX.h"

#include <cassert>

using namespace Microsoft::WRL;

CommandQueue::CommandQueue(ID3D12Device2* device, const D3D12_COMMAND_LIST_TYPE type) : m_type(type)
{
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = type;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)));

	m_fence = std::make_unique<Fence>(device);
}

CommandQueue::~CommandQueue()
= default;

// Execute the command list. Returns the fence value signaled after execution.
uint64_t CommandQueue::Execute(CommandList& commandList, const uint32_t frameIndex) const
{
    assert(commandList.m_type == m_type &&
        "Command queue and command list have incompatible types!");
    assert(frameIndex < SwapChain::m_numBuffering);

    commandList.Close();

    ID3D12CommandList* const lists[] = { commandList.m_commandList.Get() };
    m_queue->ExecuteCommandLists(1, lists);

    m_fence->Signal(*this);
    const uint64_t fenceValue = m_fence->GetWaitValue();

    commandList.m_allocatorFenceValues[frameIndex] = fenceValue;

    return fenceValue;
}

// Synchronize this command queue with another command queue up until the point where this function is called.
void CommandQueue::GPUSynchronize(const CommandQueue& queue) const
{
	ThrowIfFailed(m_queue->Wait(queue.m_fence->m_fence.Get(), queue.m_fence->GetWaitValue()));
}

// Signal fence and wait for it to be signaled.
void CommandQueue::Flush() const
{
	m_fence->Signal(*this);
	m_fence->Wait();
}
