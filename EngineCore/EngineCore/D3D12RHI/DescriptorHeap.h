#pragma once

#include "VariableSizeAllocationsManager.h"

/*
* Descriptor are structure (small block of memory) which tell shader where to find the resource, and how interprete data resource(GPU).
* "Descriptor heap" are chunk of memory (array of Descriptor) where Descriptor are stored. It could be Shader Visible(CBV, UAV, SRV, Sampler),
* Shader No Visible(RTV, DSV, ¿IBV, VBV and SOV?). GPU access == Shader Visible, CPU access == Non Shader Visible
* There are four types of descriptor heaps in D3D12:
* Constant Buffer/Shader Resource/Unordered Access view, Sampler, Render Target View, Depth Stencil View
*/
// http://diligentgraphics.com/diligent-engine/architecture/d3d12/ 
// https://www.codeproject.com/Articles/1180619/Managing-Descriptor-Heaps-in-Direct-D
namespace RHI
{
	class RenderDevice;
	class DescriptorHeapAllocation;
	class DynamicSuballocationsManager;

	class IDescriptorAllocator
	{
	public:
		virtual DescriptorHeapAllocation Allocate(UINT32 count) = 0;
		virtual void Free(DescriptorHeapAllocation&& allocation) = 0;
		virtual UINT32 GetDescriptorSize() const = 0;
	};

	/*
	* Describe the allocation of the descriptors, which is simple ranger of descriptor
	*/
	class DescriptorHeapAllocation
	{
	public:
		// Creates null allocation
		DescriptorHeapAllocation() noexcept :
			m_NumHandles{ 1 },
			m_pDescriptorHeap{ nullptr },
			m_DescriptorIncrementSize{ 0 }
		{
			m_FirstCpuHandle.ptr = 0;
			m_FirstGpuHandle.ptr = 0;
		}

		// Initializes non-null allocation 
		DescriptorHeapAllocation(IDescriptorAllocator& allocator,
			ID3D12DescriptorHeap* descriptorHeap,
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
			UINT32                      numHandles,
			UINT16                      allocationManagerID) noexcept :
			m_FirstCpuHandle{ cpuHandle },
			m_FirstGpuHandle{ gpuHandle },
			m_pAllocator{ &allocator },
			m_NumHandles{ numHandles },
			m_pDescriptorHeap{ descriptorHeap },
			m_AllocationManagerId{ allocationManagerID }
		{
			auto descriptorSize = m_pAllocator->GetDescriptorSize();
			m_DescriptorIncrementSize = static_cast<UINT16>(descriptorSize);
		}

		// Move constructor (copy is not allowed)
		DescriptorHeapAllocation(DescriptorHeapAllocation&& allocation) noexcept :
			m_FirstCpuHandle{ std::move(allocation.m_FirstCpuHandle) },
			m_FirstGpuHandle{ std::move(allocation.m_FirstGpuHandle) },
			m_NumHandles{ std::move(allocation.m_NumHandles) },
			m_pAllocator{ std::move(allocation.m_pAllocator) },
			m_AllocationManagerId{ std::move(allocation.m_AllocationManagerId) },
			m_pDescriptorHeap{ std::move(allocation.m_pDescriptorHeap) },
			m_DescriptorIncrementSize{ std::move(allocation.m_DescriptorIncrementSize) }

		{
			allocation.Reset();
		}

		// Move assignment (assignment is not allowed)
		DescriptorHeapAllocation& operator=(DescriptorHeapAllocation&& allocation) noexcept
		{
			m_FirstCpuHandle = std::move(allocation.m_FirstCpuHandle);
			m_FirstGpuHandle = std::move(allocation.m_FirstGpuHandle);
			m_NumHandles = std::move(allocation.m_NumHandles);
			m_pAllocator = std::move(allocation.m_pAllocator);
			m_AllocationManagerId = std::move(allocation.m_AllocationManagerId);
			m_pDescriptorHeap = std::move(allocation.m_pDescriptorHeap);
			m_DescriptorIncrementSize = std::move(allocation.m_DescriptorIncrementSize);

			allocation.Reset();

			return *this;
		}

		DescriptorHeapAllocation(const DescriptorHeapAllocation&) = delete;
		DescriptorHeapAllocation& operator=(const DescriptorHeapAllocation&) = delete;

		// Destructor
		~DescriptorHeapAllocation()
		{
			if (!IsNull() && m_pAllocator)
				m_pAllocator->Free(std::move(*this));
		}

		void Reset()
		{
			m_FirstCpuHandle.ptr = 0;
			m_FirstGpuHandle.ptr = 0;
			m_pAllocator = nullptr;
			m_pDescriptorHeap = nullptr;
			m_NumHandles = 0;
			m_AllocationManagerId = static_cast<UINT16>(-1);
			m_DescriptorIncrementSize = 0;
		}

		// Getters
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT32 Offset = 0) const
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_FirstCpuHandle;
			CPUHandle.ptr += m_DescriptorIncrementSize * Offset;

			return CPUHandle;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT32 Offset = 0) const
		{
			D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle = m_FirstGpuHandle;
			GPUHandle.ptr += m_DescriptorIncrementSize * Offset;

			return GPUHandle;
		}

		ID3D12DescriptorHeap* GetDescriptorHeap() { return m_pDescriptorHeap; }

		size_t GetNumHandles() const { return m_NumHandles; }
		bool IsNull() const { return m_FirstCpuHandle.ptr == 0; }
		bool IsShaderVisible() const { return m_FirstGpuHandle.ptr != 0; }
		size_t GetAllocationManagerId() const { return m_AllocationManagerId; }
		UINT GetDescriptorSize() const { return m_DescriptorIncrementSize; }

	private:
		// First CPU descriptor handle in this allocation
		D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCpuHandle = { 0 };
		// First GPU descriptor handle in this allocation
		D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGpuHandle = { 0 };

		// Pointer to the descriptor heap allocator that created this allocation
		IDescriptorAllocator* m_pAllocator = nullptr;

		// Pointer to the D3D12 descriptor heap that contains descriptors in this allocation
		ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr;

		// Number of descriptor in the allocation
		UINT32 m_NumHandles;

		// One allocator may support several allocation manager, This ID is used to find the Allocation Manager that created the Allocation
		UINT16  m_AllocationManagerId = static_cast<UINT16>(-1);

		// Descriptor Increment size, use to find a Descriptor in some Allocation
		UINT16 m_DescriptorIncrementSize = 0;
	};

	/*
	* Is the main workhorse class that manages allocations in D3D12 descriptor heap using variable-size GPU allocations manager
	* In CPU-Only Descriptor Heap, a completer descriptor is managed
	* In GPU-Only Descriptor Heap, because there is only 1 Descriptor Heap, only manages part of it
	* Use VariableSizeAllocationsManager to manage free memory in heap
	*  |  X  X  X  X  O  O  O  X  X  O  O  X  O  O  O  O  |  D3D12 descriptor heap
	*
	*  X - used descriptor
	*  O - available descriptor
	*/
	class DescriptorHeapAllocationManager
	{
	public:
		// Creates a new D3D12 descriptor heap
		DescriptorHeapAllocationManager(RenderDevice& renderDevice,
			IDescriptorAllocator& parentAllocator,
			size_t                            thisManagerId,
			const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc);

		// Uses subrange of descriptors in the existing D3D12 descriptor heap
		// that starts at offset FirstDescriptor and uses NumDescriptors descriptors
		DescriptorHeapAllocationManager(RenderDevice& renderDevice,
			IDescriptorAllocator& parentAllocator,
			size_t                thisManagerId,
			Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap,
			UINT32                firstDescriptor,
			UINT32                numDescriptors);

		DescriptorHeapAllocationManager(DescriptorHeapAllocationManager&& rhs) noexcept :
			m_ParentAllocator{ rhs.m_ParentAllocator },
			m_RenderDevice{ rhs.m_RenderDevice },
			m_ThisManagerId{ rhs.m_ThisManagerId },
			m_HeapDesc{ rhs.m_HeapDesc },
			m_DescriptorIncrementSize{ rhs.m_DescriptorIncrementSize },
			m_NumDescriptorsInAllocation{ rhs.m_NumDescriptorsInAllocation },
			m_FirstCPUHandle{ rhs.m_FirstCPUHandle },
			m_FirstGPUHandle{ rhs.m_FirstGPUHandle },
			m_MaxAllocatedNum{ rhs.m_MaxAllocatedNum },
			m_FreeBlockManager{ std::move(rhs.m_FreeBlockManager) },
			m_DescriptorHeap{ std::move(rhs.m_DescriptorHeap) }
		{
			rhs.m_NumDescriptorsInAllocation = 0;
			rhs.m_ThisManagerId = static_cast<size_t>(-1);
			rhs.m_FirstCPUHandle.ptr = 0;
			rhs.m_FirstGPUHandle.ptr = 0;
			rhs.m_MaxAllocatedNum = 0;
		}

		DescriptorHeapAllocationManager& operator = (DescriptorHeapAllocationManager&&) = delete;
		DescriptorHeapAllocationManager(const DescriptorHeapAllocationManager&) = delete;
		DescriptorHeapAllocationManager& operator = (const DescriptorHeapAllocationManager&) = delete;

		~DescriptorHeapAllocationManager();

		DescriptorHeapAllocation Allocate(UINT32 count);
		void FreeAllocation(DescriptorHeapAllocation&& allocation);

		size_t GetNumAvailableDescriptors() { return m_FreeBlockManager.GetFreeSize(); }
		UINT32 GetMaxDescriptors()         const { return m_NumDescriptorsInAllocation; }
		size_t GetMaxAllocatedSize()       const { return m_MaxAllocatedNum; }

	private:
		IDescriptorAllocator& m_ParentAllocator;
		RenderDevice& m_RenderDevice;

		// D3D12 descriptor heap object
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
		// Heap Desc
		const D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;

		// Allocations manager used to handle descriptor allocations within the heap
		VariableSizeAllocationsManager m_FreeBlockManager;

		// First CPU descriptor handle in the available descriptor range
		D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCPUHandle = { 0 };
		// First GPU descriptor handle in the available descriptor range
		D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGPUHandle = { 0 };

		// External ID assigned to this descriptor allocations manager
		size_t m_ThisManagerId = static_cast<size_t>(-1);

		const UINT m_DescriptorIncrementSize = 0;
		// Number of descriptors in the allocation. 
		// If this manager was initialized as a subrange in the existing heap,
		// this value may be different from m_HeapDesc.NumDescriptors
		UINT32 m_NumDescriptorsInAllocation = 0;

		// The maximum nunber of Descriptor that have been allocated
		size_t  m_MaxAllocatedNum = 0;
	};
}