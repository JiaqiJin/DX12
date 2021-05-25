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
	* 1- Creating the Committed Resource (CreateCommittedResource), 2 - Resource Barrier  
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

	class GpuDefaultBuffer : public GpuBuffer
	{
	public:
		GpuDefaultBuffer(UINT32 numElements, UINT32 elementSize, const void* initialData)
			: GpuBuffer(numElements, elementSize, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT)
		{
			CreateBufferResource(initialData);
		}
		
		GpuDefaultBuffer(UINT32 numElements, UINT32 elementSize, const GpuUploadBuffer& srcData, UINT32 srcOffset)
			: GpuBuffer(numElements, elementSize, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT)
		{
			CreateBufferResource(srcData, srcOffset);
		}
	};

	// Upload heaps must start  out in the state D3D12_RESOURCE_STATE_GENERIC_READ
	class GpuUploadBuffer : public GpuBuffer
	{
	public:
		GpuUploadBuffer(UINT32 numElements, UINT32 elementSize)
			: GpuBuffer(numElements, elementSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD)
		{
			CreateBufferResource(nullptr);
		}

		void* Map()
		{
			void* memory;
			m_pResource->Map(0, &CD3DX12_RANGE(0, m_bufferSize), &memory);
			return memory;
		}

		void Unmap(size_t begin = 0, size_t end = -1)
		{
			m_pResource->Unmap(0, &CD3DX12_RANGE(begin, std::min(end, m_bufferSize)));
		}
	};

	class GpuDynamicBuffer : public GpuBuffer
	{
	public:
		GpuDynamicBuffer(UINT32 numElements, UINT32 elementSize)
			: GpuBuffer(numElements, elementSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD)
		{

		}

		virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const override;

		void* Map(CommandContext& cmdContext, size_t alignment);

	protected:
		// No need to release, released by DynamicResourceHeap in every frame
		D3D12DynamicAllocation m_DynamicData;
	};
}