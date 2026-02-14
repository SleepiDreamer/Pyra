#include "RTPipeline.h"

using namespace Microsoft::WRL;

RTPipeline::RTPipeline()
{

}

RTPipeline::~RTPipeline() = default;

D3D12_DISPATCH_RAYS_DESC RTPipeline::GetDispatchRaysDesc() const
{
	return D3D12_DISPATCH_RAYS_DESC{};
}
