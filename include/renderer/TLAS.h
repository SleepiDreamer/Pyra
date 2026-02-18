#pragma once
#include "GPUBuffer.h"
#include "CommonDX.h"

class GPUAllocator;
class CommandQueue;

class TLAS
{
public:
	TLAS(RenderContext& context);
	~TLAS();
	TLAS(const TLAS&) = delete;
	TLAS& operator=(const TLAS&) = delete;

	void Build(ID3D12Device10* device, const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances);
	void Update(const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances) const;

	[[nodiscard]] const GPUBuffer& GetResource() const { return m_result; }
private:
	RenderContext& m_context;
	GPUBuffer m_result;
	GPUBuffer m_scratch;
	GPUBuffer m_tlasInstances;
	D3D12_RAYTRACING_INSTANCE_DESC* m_instanceData = nullptr;
};

