#pragma once
#include "GPUBuffer.h"
#include "CommonDX.h"

class GPUAllocator;
class CommandQueue;

class Model
{
public:
    Model(GPUAllocator& allocator, CommandQueue& commandQueue, const std::string& name);
    ~Model();

private:
	GPUBuffer m_vertexBuffer;
	GPUBuffer m_indexBuffer;
	std::string name;
};