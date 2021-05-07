#pragma once
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
		virtual void Free(DescriptorHeapAllocation& allocation) = 0;
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
	*/
	class DescriptorHeapAllocationManager
	{
	public:

	};

	/*
	* Implement CPU-only descriptor heap that is used as storage of resource view descriptor hanle
	*/
	class CPUDescriptorHeap
	{
	public:

	};

	/*
	* Implement shader-visible descriptor heap thta holds descriptor handles use by GPU commands.
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