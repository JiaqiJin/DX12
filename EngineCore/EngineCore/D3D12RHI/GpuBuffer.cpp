#include "../pch.h"
#include "GpuBuffer.h"
#include "RenderDevice.h"
#include "CommandContext.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	// -------------------------- GPU BUFFER ---------------------------------------
	GpuBuffer::GpuBuffer(UINT32 numElements, UINT32 elementSize, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType)
	{
		m_ElementCount = numElements;
		m_ElementSize = elementSize;
		m_BufferSize = elementSize * numElements;
		m_UsageState = initialState;
		m_HeapType = heapType;
	}

	void GpuBuffer::CreateBufferResource(const void* initData)
	{
		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = m_HeapType;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ID3D12Device* device = RenderDevice::GetSingleton().GetD3D12Device();
		ThrowIfFailed(device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

		// If initData is provided, the data will uploaded to the upload heap, and then copy to buffer
		if (initData != nullptr)
			CommandContext::InitializeBuffer(*this, initData, m_BufferSize);
	}

	void GpuBuffer::CreateBufferResource(const GpuUploadBuffer& srcData, UINT32 srcOffset)
	{
		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();
		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = m_HeapType;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ID3D12Device* d3dDevice = RenderDevice::GetSingleton().GetD3D12Device();
		ThrowIfFailed(d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

		// If initData is provided, the data will uploaded to the upload heap, and then copy to buffer
		CommandContext::InitializeBuffer(*this, srcData, srcOffset);
	}

	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::CreateVBV(size_t offset, uint32_t size, uint32_t stride) const
	{
		D3D12_VERTEX_BUFFER_VIEW VBView = {};
		VBView.BufferLocation = m_GpuVirtualAddress + offset;
		VBView.SizeInBytes = size;
		VBView.StrideInBytes = stride;
		return VBView;
	}

	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::CreateVBV(size_t baseVertexIndex /*= 0*/) const
	{
		size_t offset = baseVertexIndex * m_ElementSize;
		return CreateVBV(offset, (uint32_t)(m_BufferSize - offset), m_ElementSize);
	}

	D3D12_INDEX_BUFFER_VIEW GpuBuffer::CreateIBV(size_t offset, uint32_t size, bool b32Bit /*= false*/) const
	{
		D3D12_INDEX_BUFFER_VIEW IBView = {};
		IBView.BufferLocation = m_GpuVirtualAddress + offset;
		IBView.SizeInBytes = size;
		IBView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		return IBView;
	}

	D3D12_INDEX_BUFFER_VIEW GpuBuffer::CreateIBV(size_t startIndex /*= 0*/) const
	{
		size_t offset = m_ElementSize * startIndex;
		return CreateIBV(offset, (uint32_t)(m_BufferSize - offset), m_ElementSize == 4);
	}

	std::shared_ptr<GpuResourceDescriptor> GpuBuffer::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = m_ElementCount;
		SRVDesc.Buffer.StructureByteStride = m_ElementSize;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuBuffer::CreateUAV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());
		// D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		// TODO
		// 		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		// 		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		// 		UAVDesc.Buffer.CounterOffsetInBytes = 0;
		// 		UAVDesc.Buffer.NumElements = m_ElementCount;
		// 		UAVDesc.Buffer.StructureByteStride = m_ElementSize;
		// 		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		// 
		// 		RenderDevice::GetSingleton().GetD3D12Device()->CreateUnorderedAccessView(m_pResource.Get(), &UAVDesc, descriptor->GetCpuHandle());
		return descriptor;
	}

	D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer()
	{
		assert(m_BufferSize != 0);

		D3D12_RESOURCE_DESC Desc = {};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		// Buffer format is unknown by default
		Desc.Format = DXGI_FORMAT_UNKNOWN;
		Desc.Height = 1;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		Desc.MipLevels = 1;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)m_BufferSize;

		return Desc;
	}

	D3D12_GPU_VIRTUAL_ADDRESS GpuDynamicBuffer::GetGpuVirtualAddress() const
	{
		return m_DynamicData.GPUAddress;
	}

	void* GpuDynamicBuffer::Map(CommandContext& cmdContext, size_t alignment)
	{
		m_DynamicData = cmdContext.AllocateDynamicSpace(m_BufferSize, alignment);
		return m_DynamicData.CPUAddress;
	}
}