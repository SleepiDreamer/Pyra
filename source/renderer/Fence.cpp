//#include "Fence.h"
//#include "CommandQueue.h"
//#include "CommonDX.h"
//
//#include <d3d12.h>
//#include <d3dx12.h>
//
//Fence::Fence(ID3D12Device2* device)
//{
//    ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
//    m_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
//    if (!m_event)
//    {
//        throw std::runtime_error("Failed to create fence event handle!");
//    }
//}
//
//Fence::~Fence()
//{
//    if (m_event)
//    {
//        CloseHandle(m_event);
//        m_event = nullptr;
//    }
//}
//
//void Fence::Signal(const CommandQueue& queue)
//{
//    m_fenceValue++;
//    ThrowIfFailed(queue.m_queue->Signal(m_fence.Get(), m_fenceValue));
//}
//
//void Fence::Wait() const
//{
//	WaitFor(m_fenceValue);
//}
//
//void Fence::WaitFor(const uint64_t value) const
//{
//    if (m_fence->GetCompletedValue() >= value)
//    {
//        return;
//    }
//
//    ThrowIfFailed(m_fence->SetEventOnCompletion(value, m_event));
//    WaitForSingleObject(m_event, INFINITE);
//}
//
//bool Fence::IsComplete(const uint64_t value) const
//{
//    return m_fence->GetCompletedValue() >= value;
//}
//
//uint64_t Fence::GetWaitValue() const { return m_fenceValue; }
//
//uint64_t Fence::GetCurrentValue() const { return m_fence->GetCompletedValue(); }
