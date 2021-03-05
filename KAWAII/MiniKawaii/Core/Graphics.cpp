#include "Graphics.h"
#include "CommandListManager.h"

using namespace Rendering;

CommandListManager Graphics::s_CommandManager;
ID3D12Device* Graphics::s_Device = nullptr;

DescriptorAllocator Graphics::s_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
};