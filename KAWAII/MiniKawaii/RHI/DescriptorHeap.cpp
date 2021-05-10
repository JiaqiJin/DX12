#include "../pch.h"
#include "DescriptorHeap.h"
#include "RenderDevice.h"

namespace RHI
{
	DescriptorHeapAllocationManager::DescriptorHeapAllocationManager(RenderDevice& renderDevice,
		IDescriptorAllocator& parentAllocator,
		size_t thisManagerId,
		const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc) :
		m_ParentAllocator{ parentAllocator },
		m_RenderDevice{ renderDevice },
		m_ThisManagerId{ thisManagerId },
		m_HeapDesc{ heapDesc },
		m_DescriptorIncrementSize{ m_RenderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(heapDesc.Type) },
		m_NumDescriptorsInAllocation{ heapDesc.NumDescriptors },
		m_FreeBlockManager{ heapDesc.NumDescriptors }
	{
		auto* d3d12Device = m_RenderDevice.GetD3D12Device();

		m_FirstCPUHandle.ptr = 0;
		m_FirstGPUHandle.ptr = 0;

		d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap));
		m_FirstCPUHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
			m_FirstGPUHandle = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}

	DescriptorHeapAllocationManager::DescriptorHeapAllocationManager(RenderDevice& renderDevice,
		IDescriptorAllocator& parentAllocator,
		size_t thisManagerId,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		UINT32 firstDescriptor,
		UINT32 numDescriptors) :
		m_ParentAllocator{ parentAllocator },
		m_RenderDevice{ renderDevice },
		m_ThisManagerId{ thisManagerId },
		m_HeapDesc{ descriptorHeap->GetDesc() },
		m_DescriptorIncrementSize{ renderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(m_HeapDesc.Type) },
		m_NumDescriptorsInAllocation{ numDescriptors },
		m_FreeBlockManager{ numDescriptors },
		m_DescriptorHeap{ descriptorHeap }
	{
		m_FirstCPUHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_FirstCPUHandle.ptr += m_DescriptorIncrementSize * firstDescriptor;

		if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			m_FirstGPUHandle = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			m_FirstGPUHandle.ptr += m_DescriptorIncrementSize * firstDescriptor;
		}
	}

	DescriptorHeapAllocationManager::~DescriptorHeapAllocationManager()
	{
		assert(m_FreeBlockManager.GetFreeSize() == m_NumDescriptorsInAllocation && "Not all descriptors were released");
	}

	DescriptorHeapAllocation DescriptorHeapAllocationManager::Allocate(UINT32 count)
	{
		// Descriptor does not need to be aligned, pass in 1
		auto allocation = m_FreeBlockManager.Allocate(count, 1);
		if (!allocation.IsValid())
			return DescriptorHeapAllocation();

		assert(allocation.size == count);
		
		// Compute the first CPU and GPU descriptor handles in the allocation by
		// offseting the first CPU and GPU descriptor handle in the range
		auto CPUHandle = m_FirstCPUHandle;
		CPUHandle.ptr += allocation.unalignedOffset * m_DescriptorIncrementSize;

		auto GPUHandle = m_FirstGPUHandle;
		GPUHandle.ptr += allocation.unalignedOffset * m_DescriptorIncrementSize;

		m_MaxAllocatedNum = std::max(m_MaxAllocatedNum, m_FreeBlockManager.GetUsedSize());

		return DescriptorHeapAllocation(m_ParentAllocator, m_DescriptorHeap.Get(), CPUHandle, GPUHandle,
			count, static_cast<UINT16>(m_ThisManagerId));
	}

	void DescriptorHeapAllocationManager::FreeAllocation(DescriptorHeapAllocation&& allocation)
	{
		assert((allocation.GetAllocationManagerId() == m_ThisManagerId) && "Invalid descriptor heap manager Id");

		if (allocation.IsNull())
			return;

		auto descriptorOffset = (allocation.GetCpuHandle().ptr - m_FirstCPUHandle.ptr) / m_DescriptorIncrementSize;
		m_FreeBlockManager.Free(descriptorOffset, allocation.GetNumHandles());

		allocation.Reset();
	}
}