#include "Texture.h"
#include "GPUAllocator.h"
#include "UploadContext.h"

void Texture::Create(const RenderContext& context, const void* data, const uint32_t width, const uint32_t height, const DXGI_FORMAT format, const std::string& name)
{
	m_resource = context.allocator->CreateTexture(
		width, height, format,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_FLAG_NONE,
		ToWideString(name.c_str()).c_str());

	context.uploadContext->UploadTexture(m_resource, data, width, height, format);

	 m_srv = context.descriptorHeap->Allocate();
	   
	 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	 srvDesc.Format = format;
	 srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	 srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	 srvDesc.Texture2D.MipLevels = 1;
	 context.device->CreateShaderResourceView(m_resource.resource, &srvDesc, m_srv.cpuHandle);
}
