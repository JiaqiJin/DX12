#pragma once

#include "GpuResource.h"

// https://docs.microsoft.com/en-us/windows/win32/direct3d12/uploading-resources

namespace RHI
{
	class GpuUploadBuffer;
	class GpuResourceDescriptor;
	class CommandContext;

	/*
	* Default state : GPU READ/WRITE
	* Upload state : CPU WRITE, GPU READ
	* Dynamic state : Dynamic buffer, CPU READ, GPU WRITE
	* 1- Creating the Committed Resource (CreateCommittedResource), 2 - Resource Barrier
	*/
	class GpuBuffer : public GpuResource
	{
	public:
		GpuBuffer(UINT32 numElements, UINT32 elementSize, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType);

		// Descriptor
		virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

		// Offet : start address offset, size : buffer size, stride : size of each element
		D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t offset, uint32_t size, uint32_t stride) const;
		D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t baseVertexIndex = 0) const;

		D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t offset, uint32_t size, bool b32Bit = false) const;
		D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t startIndex = 0) const;
		
		// Getters
		UINT64 GetBufferSize() const { return m_BufferSize; }
		UINT32 GetElementCount() const { return m_ElementCount; }
		UINT32 GetElementSize() const { return m_ElementSize; }

	protected:
		// Create Buffer resources
		void CreateBufferResource(const void* initData);
		void CreateBufferResource(const GpuUploadBuffer& srcData, UINT32 srcOffset);

		D3D12_RESOURCE_DESC DescribeBuffer();

		// address of GPU buffer
		D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		D3D12_HEAP_TYPE m_HeapType;

		UINT64 m_BufferSize;
		UINT32 m_ElementCount;
		UINT32 m_ElementSize;
	};

	class GpuDefaultBuffer : public GpuBuffer
	{
	public:
		GpuDefaultBuffer(UINT32 numElements, UINT32 elementSize, const void* initialData)
			: GpuBuffer(numElements, elementSize, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT) 
		{
			CreateBufferResource(initialData);
		}

		GpuDefaultBuffer(UINT32 NumElements, UINT32 ElementSize, const GpuUploadBuffer& srcData, UINT32 srcOffset) :
			GpuBuffer(NumElements, ElementSize, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT)
		{
			CreateBufferResource(srcData, srcOffset);
		}
	};

	class GpuUploadBuffer : public GpuBuffer
	{
	public:
		GpuUploadBuffer(UINT32 numElement, UINT32 elementSize)
			: GpuBuffer(numElement, elementSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD)
		{
			CreateBufferResource(nullptr);
		}

		void* Map(void)
		{
			void* Memory;
			m_pResource->Map(0, &CD3DX12_RANGE(0, m_BufferSize), &Memory);
			return Memory;
		}

		void UnMap(size_t begin = 0, size_t end = -1)
		{
			m_pResource->Unmap(0, &CD3DX12_RANGE(begin, std::min(end, m_BufferSize)));
		}
	};
}