#pragma once
#include "VariableSizeAllocationsManager.h"

/*Descriptor are structure(small block of memory) which tell shader where to find the resource, and how interprete data resource(GPU).
* Descriptor heap are chunk of memory where Descriptor are stored. It could be Shader Visible(CBV, UAV, SRV, Sampler), 
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
		// Allocate count Descriptors
		virtual DescriptorHeapAllocation Allocate(UINT32 count) = 0;
		virtual void Free(DescriptorHeapAllocation&& allocation) = 0;
		virtual UINT32 GetDescriptorSize() const = 0;
	};

	/*
	* Descripe the allocation of the descriptors, which is simple ranger of descriptor
	*/
	class DescriptorHeapAllocation
	{
	public:
		// Creates null allocation
		DescriptorHeapAllocation() noexcept :
			m_NumHandles{ 1 },
			m_pDescriptorHeap{ nullptr },
			m_DescriptorIncrementSize { 0 }
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

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT32 Offset = 0) const
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CPUhandle = m_FirstCpuHandle;
			CPUhandle.ptr += m_DescriptorIncrementSize * Offset;

			return CPUhandle;
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

		IDescriptorAllocator* m_pAllocator = nullptr;
		ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr;

		// Number of descriptors in the allocation
		UINT32 m_NumHandles = 0;

		// One allocator may support several allocation manager, This ID is used to find the Allocation Manager that created the Allocation
		UINT16 m_AllocationManagerId = static_cast<UINT16>(-1);

		// Descriptor Increment size
		UINT16 m_DescriptorIncrementSize = 0;
	};

	/*
	* Is the main workhorse class that manages allocations in D3D12 descriptor heap using variable-size GPU allocations manager
	* In CPU-Only Descriptor Heap, a completer descriptor is managed
	* In GPU-Only Descriptor Heap, because there is only 1 DH, only manages part of it
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

		size_t m_ThisManagerId = static_cast<size_t>(-1);

		const D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;

		const UINT m_DescriptorIncrementSize = 0;

		// The total number of Descriptors that can be allocated
		UINT32 m_NumDescriptorsInAllocation = 0;

		VariableSizeAllocationsManager m_FreeBlockManager;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCPUHandle = { 0 };
		D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGPUHandle = { 0 };

		size_t m_MaxAllocatedNum = 0;
	};

	/*
	* Implement CPU-only descriptor heap that is used as storage of resource view descriptor hanle
	* Every CPU descriptor heap keeps a pool of Descriptor Heap Allocation Managers and a list of managers that have unused descriptors:
	*        m_HeapPool[0]                m_HeapPool[1]                 m_HeapPool[2]
    * |  X  X  X  X  X  X  X  X |, |  X  X  X  O  O  X  X  O  |, |  X  O  O  O  O  O  O  O  |
	*  X - used descriptor                m_AvailableHeaps = {1,2}
    *  O - available descriptor
	* Render Device contains four CPUDescriptorHeap objects, corresponding to the four Descriptor Heaps of D3D12
	* (SRV_CBV_UAV, Sampler, RTV, DSV).
	*/
	class CPUDescriptorHeap final : public IDescriptorAllocator
	{
	public:
		CPUDescriptorHeap(RenderDevice& renderDevice,
			UINT32 numDescriptorsInHeap,
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			D3D12_DESCRIPTOR_HEAP_FLAGS flags);

		CPUDescriptorHeap(const CPUDescriptorHeap&) = delete;
		CPUDescriptorHeap(CPUDescriptorHeap&&) = delete;
		CPUDescriptorHeap& operator = (const CPUDescriptorHeap&) = delete;
		CPUDescriptorHeap& operator = (CPUDescriptorHeap&&) = delete;

		~CPUDescriptorHeap();

		virtual DescriptorHeapAllocation Allocate(uint32_t count) override final;
		virtual void Free(DescriptorHeapAllocation&& allocation) override final;
		virtual UINT32 GetDescriptorSize() const override final { return m_DescriptorSize; }

	private:
		void FreeAllocation(DescriptorHeapAllocation&& allocation);

		RenderDevice& m_RenderDevice;

		std::vector<DescriptorHeapAllocationManager> m_HeapPool;
		std::unordered_set<size_t, std::hash<size_t>, std::equal_to<size_t>> m_AvailableHeaps;

		D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
		const UINT                 m_DescriptorSize = 0;

		UINT32 m_MaxSize = 0;
		UINT32 m_CurrentSize = 0;
	};

	/*
	* Implement shader-visible descriptor heap thta holds descriptor handles use by GPU commands.
	* For GPU the must access a descriptor, they must reside in a shader visible descriptor heap(SRV_CBV_UAV and SAMPLER).
	* " GPUDescriptorHeap object contains only single D3D12 descriptor heap ".
	* The space is broken in 2 part : 1- keep rarely changing descriptor handle(corresponding to static and mutable variables),
	* 2 - used to hold the dynamic descriptor handle(everal threads record commands simultaneously == problem bottleneck). 
	* Fisrt stage : command contex allocates a chunck of descriptor from the shared dynamic part of GPU descriptor heap.
	* Second stage : is suballoction from that chunk.
	* 
    *   static and mutable handles      ||                 dynamic space
    *                                   ||    chunk 0     chunk 1     chunk 2     unused
    *| X O O X X O X O O O O X X X X O  ||  | X X X O | | X X O O | | O O O O |  O O O O  ||
    *                                             |         |
    *                                   suballocation       suballocation
    *                                  within chunk 0       within chunk 1
	* Render Device contains two GPUDescriptorHeap objects (SRV_CBV_UAV and Sampler)
	* 
	* Device Context is used to allocate Dynamic Resource Descriptor
	*   _______________________________________________________________________________________________________________________________
    * | Render Device                                                                                                                 |
    * |                                                                                                                               |
    * | m_CPUDescriptorHeaps[CBV_SRV_UAV] |  X  X  X  X  X  X  X  X  |, |  X  X  X  X  X  X  X  X  |, |  X  O  O  X  O  O  O  O  |    |
    * | m_CPUDescriptorHeaps[SAMPLER]     |  X  X  X  X  O  O  O  X  |, |  X  O  O  X  O  O  O  O  |                                  |
    * | m_CPUDescriptorHeaps[RTV]         |  X  X  X  O  O  O  O  O  |, |  O  O  O  O  O  O  O  O  |                                  |
    * | m_CPUDescriptorHeaps[DSV]         |  X  X  X  O  X  O  X  O  |                                                                |
    * |                                                                               ctx1        ctx2                                |
    * | m_GPUDescriptorHeaps[CBV_SRV_UAV]  | X O O X X O X O O O O X X X X O  ||  | X X X O | | X X O O | | O O O O |  O O O O  ||    |
    * | m_GPUDescriptorHeaps[SAMPLER]      | X X O O X O X X X O O X O O O O  ||  | X X O O | | X O O O | | O O O O |  O O O O  ||    |
    * |                                                                                                                               |
    * |_______________________________________________________________________________________________________________________________|
    *
    *  ________________________________________________               ________________________________________________
    * |Device Context 1                                |             |Device Context 2                                |
    * |                                                |             |                                                |
    * | m_DynamicGPUDescriptorAllocator[CBV_SRV_UAV]   |             | m_DynamicGPUDescriptorAllocator[CBV_SRV_UAV]   |
    * | m_DynamicGPUDescriptorAllocator[SAMPLER]       |             | m_DynamicGPUDescriptorAllocator[SAMPLER]       |
    * |________________________________________________|             |________________________________________________|
	*/
	class GPUDescriptorHeap
	{
	public:

	};

	/*
	* Responsible to allocate short living dynamic descriptor handle use in the current frame only
	*/
	class DynamicSuballocationsManager
	{
	public:

	};
}