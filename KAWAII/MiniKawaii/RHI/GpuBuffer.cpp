#include "../pch.h"
#include "GpuBuffer.h"
#include "RenderDevice.h"
#include "CommandContext.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	GpuBuffer::GpuBuffer(UINT32 NumElements, UINT32 ElementSize, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType)
	{
		m_elementCount = NumElements;
		m_elementSize = ElementSize;
		m_bufferSize = NumElements * ElementSize;
		m_UsageState = initialState;
		m_heapType = heapType;
	}

	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::CreateVBV(size_t baseVertexIndex /*= 0*/) const
	{
		size_t Offset = baseVertexIndex * m_elementSize;
		return CreateVBV(Offset, (uint32_t)(m_bufferSize - Offset), m_elementSize);
	}

	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::CreateVBV(size_t Offset, uint32_t size, uint32_t stride) const
	{
		D3D12_VERTEX_BUFFER_VIEW VBView;
		VBView.BufferLocation = m_GpuVirtualAddress + Offset;
		VBView.SizeInBytes = size;
		VBView.StrideInBytes = stride;
		return VBView;
	}

	D3D12_INDEX_BUFFER_VIEW GpuBuffer::CreateIBV(size_t startIndex /*= 0*/) const
	{
		size_t offset = startIndex * m_elementSize;
		return CreateIBV(offset, (uint32_t)(m_bufferSize - offset), m_elementSize == 4);
	}

	D3D12_INDEX_BUFFER_VIEW GpuBuffer::CreateIBV(size_t offset, uint32_t size, bool b32Bit) const
	{
		D3D12_INDEX_BUFFER_VIEW IBView;
		IBView.BufferLocation = m_GpuVirtualAddress + offset;
		IBView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		IBView.SizeInBytes = size;
		return IBView;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuBuffer::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = m_elementCount;
		srvDesc.Buffer.StructureByteStride = m_elementSize;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &srvDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuBuffer::CreateUAV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());
		// TODO UAV need Counter Buffer
		return descriptor;
	}

	void GpuBuffer::CreateBufferResource(const void* initialData)
	{
		D3D12_RESOURCE_DESC resourceDesc = DescribeBuffer();
		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = m_heapType;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ID3D12Device* d3dDevice = RenderDevice::GetSingleton().GetD3D12Device();
		ThrowIfFailed(d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

		if(initialData)
			CommandContext::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	void GpuBuffer::CreateBufferResource(const GpuUploadBuffer& srcData, UINT32 srcOffset)
	{
		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();
		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = m_heapType;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ID3D12Device* d3dDevice = RenderDevice::GetSingleton().GetD3D12Device();
		ThrowIfFailed(d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

		CommandContext::InitializeBuffer(*this, srcData, srcOffset);
	}

	D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer()
	{
		assert(m_bufferSize != 0);

		D3D12_RESOURCE_DESC Desc{};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		Desc.Format = DXGI_FORMAT_UNKNOWN; // Buffer format is unknown
		Desc.Height = 1;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		Desc.MipLevels = 1;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)m_bufferSize;

		return Desc;
	}

	D3D12_GPU_VIRTUAL_ADDRESS GpuDynamicBuffer::GetGpuVirtualAddress() const
	{
		return m_DynamicData.GPUAddress;
	}

	// During the mapping, memory will allocated in a dynamic resource heap. UnMap is not needed
	// Constant Buffer is 256-byte aligned, the other 16-byte aligned
	void* GpuDynamicBuffer::Map(CommandContext& cmdContext, size_t alignment)
	{
		m_DynamicData = cmdContext.AllocateDynamicSpace(m_bufferSize, alignment);
		return m_DynamicData.CPUAddress;
	}
}