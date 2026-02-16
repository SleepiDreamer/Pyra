#pragma once
#include "GPUBuffer.h"
#include "CommonDX.h"

class GPUAllocator;
class CommandQueue;

class TLAS
{
public:
	TLAS(ID3D12Device5* device, GPUAllocator& allocator, CommandQueue& commandQueue);
	~TLAS();
	TLAS(const TLAS&) = delete;
	TLAS& operator=(const TLAS&) = delete;

	void Build(ID3D12Device5* device, const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances);
	void Update(const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances) const;
private:
	GPUAllocator& m_allocator;
	CommandQueue& m_commandQueue;
	GPUBuffer m_result;
	GPUBuffer m_scratch;
	GPUBuffer m_tlasInstances;
	D3D12_RAYTRACING_INSTANCE_DESC* m_instanceData = nullptr;
};

