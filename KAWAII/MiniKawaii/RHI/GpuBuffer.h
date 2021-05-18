#pragma once

#include "GpuResource.h"
#include "DynamicResource.h"

namespace RHI
{
	class CommandContext;
	class GpuUploadBuffer;
	class GpuResourceDescriptor;
	/*
	* Default state : GPU READ/WRITE
	* Upload state : CPU WRITE, GPU READ
	* Dynamic state : Dynamic buffer, CPU READ, GPU WRITE
	*/
	class GpuBuffer : public GpuResource
	{
	public:
		GpuBuffer(UINT32 NumElements, UINT32 ElementSize, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType);

		virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

		D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t offset, uint32_t size, uint32_t stride) const;
		D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t baseVertexIndex) const;

		D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t offset, uint32_t size, bool b32Bit = false) const;
		D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t startIndex = 0) const;

		virtual std::shared_ptr<GpuResourceDescriptor> CreateSRV();
		virtual std::shared_ptr<GpuResourceDescriptor> CreateUAV();
		
		// Getters
		UINT64 GetBufferSize() const { return m_bufferSize; }
		UINT32 GetElementCount() const { return m_elementCount; }
		UINT32 GetElementSize() const { return m_elementSize; }

	protected:
		void CreateBufferResource(const void* initialData);
		void CreateBufferResource(const GpuUploadBuffer& srcData, UINT32 srcOffset);

		D3D12_RESOURCE_DESC DescribeBuffer();	

		// GPU Buffer address
		D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		D3D12_HEAP_TYPE m_heapType;

		UINT32 m_elementCount;
		UINT32 m_elementSize;
		UINT64 m_bufferSize;
	};
}