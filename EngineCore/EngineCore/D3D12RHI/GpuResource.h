#pragma once

/*
* The base class of all resources on the GPU
* GPU resources include :
* - GpuBuffer : StructuredBuffer, ByteAddressBuffer, IndirectArgsBuffer, ReadbackBuffer, TypedBuffer
* - PixelBuffer: ColorBuffer, DepthBuffer
* - UploadBuffer
* - Texture
* GpuResource inherits enable_shared_from_this, because GpuResourceDescriptor will be created internally,
* and GpuResourceDescriptor will have the ownership of GpuResource,
* To ensure that GpuResource will not be released when using GpuResourceDescriptor
* https://docs.microsoft.com/en-us/windows/win32/direct3d12/resource-binding-flow-of-control
*/
namespace RHI
{
	class RenderDevice;

	class GpuResource : public std::enable_shared_from_this<GpuResource>
	{
		friend class CommandContext;
		friend class GraphicsContext;
		friend class ComputeContext;
	public:
		virtual ~GpuResource();

		void SetName(const std::wstring& name) { m_pResource->SetName(name.c_str()); }

		// Getter
		ID3D12Resource* GetResource() { return m_pResource.Get(); }
		const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
		D3D12_RESOURCE_STATES m_UsageState;
		D3D12_RESOURCE_STATES m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
	};
}