#include "../pch.h"
#include "DescriptorHeap.h"
#include "RenderDevice.h"

namespace RHI
{
	// ---------------- Descriptor HeapAllocation Manager -------------------------
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
		auto* d3d12Device = renderDevice.GetD3D12Device();

		m_FirstCPUHandle.ptr = 0;
		m_FirstGPUHandle.ptr = 0;

		d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap));
		m_FirstCPUHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
			m_FirstGPUHandle = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}

	// Use the subrange of descriptor in the existen D3D12 Descriptor Heap
	// that start at offset FirstDescriptor and uses NumDescriptor descriptors
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
		// Use variable-size GPU allocations manager to allocate the requested number of descriptors
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

		// The maximum number of Descriptors allocated
		m_MaxAllocatedNum = std::max(m_MaxAllocatedNum, m_FreeBlockManager.GetUsedSize());

		return DescriptorHeapAllocation(m_ParentAllocator, m_DescriptorHeap.Get(), CPUHandle, GPUHandle, count, static_cast<UINT16>(m_ThisManagerId));
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

	// ---------------------- CPU DESCRIPTOR HEAP --------------------
	CPUDescriptorHeap::CPUDescriptorHeap(RenderDevice& renderDevice,
		UINT32 numDescriptorsInHeap,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
		m_RenderDevice{renderDevice},
		m_HeapDesc{ type, numDescriptorsInHeap, flags, 1/* NodeMask */ },
		m_DescriptorSize{ renderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(type) }
	{
		// Creating AllocationManager
		m_HeapPool.emplace_back(renderDevice, *this, 0, m_HeapDesc);
		m_AvailableHeaps.insert(0);
	}

	CPUDescriptorHeap::~CPUDescriptorHeap()
	{
		assert((m_CurrentSize == 0) && "Not all allocations released");
		assert((m_AvailableHeaps.size() == m_HeapPool.size()) && "Not all descriptor heap pools are released");
	}

	// When allocating a new descriptor, the CPUDescriptorHeap goes throght the list of managers that have avaliable descriptor and try 
	// to process the request. If there are no avaliable managers to handle the request, create a new descriptor heap manager.
	DescriptorHeapAllocation CPUDescriptorHeap::Allocate(uint32_t count)
	{
		DescriptorHeapAllocation allocation;

		// Go through all descriptor heap managers that have free descriptors
		auto availableHeapIt = m_AvailableHeaps.begin();
		while (availableHeapIt != m_AvailableHeaps.end())
		{
			// For vector and deque, erase may cause iterator and reference pointing to other elements to become invalid,
			// but for all other containers, iterator and reference pointing to other elements will always remain valid
			auto nextIt = availableHeapIt;
			++nextIt;

			// Try to use the current Manager to allocate Descriptor
			allocation = m_HeapPool[*availableHeapIt].Allocate(count);
			// Remove the manager from the pool if it has no more available descriptors
			if (m_HeapPool[*availableHeapIt].GetNumAvailableDescriptors() == 0)
				m_AvailableHeaps.erase(*availableHeapIt);

			// Terminate the loop if descriptor was successfully allocated, otherwise
			// go to the next manager
			if (!allocation.IsNull())
				break;

			availableHeapIt = nextIt;
		}

		// If there were no available descriptor heap managers or no manager was able 
		// to suffice the allocation request, create a new manager
		if (allocation.IsNull())
		{
			if (count > m_HeapDesc.NumDescriptors)
			{
				LOG("Increasing the number of descriptors in the heap");
			}

			// Make sure the heap is large enough to accomodate the requested number of descriptors
			m_HeapDesc.NumDescriptors = std::max(m_HeapDesc.NumDescriptors, static_cast<UINT>(count));
			// Create a new descriptor heap manager. Note that this constructor creates a new D3D12 descriptor
			// heap and references the entire heap. Pool index is used as manager ID
			m_HeapPool.emplace_back(m_RenderDevice, *this, m_HeapPool.size(), m_HeapDesc);
			m_AvailableHeaps.insert(m_HeapPool.size() - 1);

			allocation = m_HeapPool[m_HeapPool.size() - 1].Allocate(count);
		}

		m_CurrentSize += static_cast<UINT32>(allocation.GetNumHandles());
		m_MaxSize = std::max(m_MaxSize, m_CurrentSize);

		return allocation;
	}

	void CPUDescriptorHeap::Free(DescriptorHeapAllocation&& allocation)
	{
		struct StaleAllocation
		{
			DescriptorHeapAllocation Allocation;
			CPUDescriptorHeap* Heap;

			StaleAllocation(DescriptorHeapAllocation&& _Allocation, CPUDescriptorHeap& _Heap)noexcept :
				Allocation{ std::move(_Allocation) },
				Heap{ &_Heap }
			{
			}

			StaleAllocation(const StaleAllocation&) = delete;
			StaleAllocation& operator= (const StaleAllocation&) = delete;
			StaleAllocation& operator= (StaleAllocation&&) = delete;

			StaleAllocation(StaleAllocation&& rhs)noexcept :
				Allocation{ std::move(rhs.Allocation) },
				Heap{ rhs.Heap }
			{
				rhs.Heap = nullptr;
			}

			~StaleAllocation()
			{
				if (Heap != nullptr)
					Heap->FreeAllocation(std::move(Allocation));
			}
		};

		m_RenderDevice.SafeReleaseDeviceObject(StaleAllocation{ std::move(allocation), *this });
	}

	void CPUDescriptorHeap::FreeAllocation(DescriptorHeapAllocation&& allocation)
	{
		auto managerID = allocation.GetAllocationManagerId();
		m_CurrentSize -= static_cast<UINT32>(allocation.GetNumHandles());
		m_HeapPool[managerID].FreeAllocation(std::move(allocation));
		m_AvailableHeaps.insert(managerID);
	}

	// ---------------------------- GPU DESCRIPTOR HEAP -------------------------
	GPUDescriptorHeap::GPUDescriptorHeap(RenderDevice& renderDevice,
		UINT32 numDescriptorsInHeap,
		UINT32 numDynamicDescriptors,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
	m_RenderDevice{ renderDevice },
	m_HeapDesc
	{
		type,
		numDescriptorsInHeap + numDynamicDescriptors,
		flags,
		1 // NodeMask
	},

	m_DescriptorHeap
	{
		[&]
		{
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
			ThrowIfFailed(renderDevice.GetD3D12Device()->CreateDescriptorHeap(&m_HeapDesc, IID_PPV_ARGS(&descriptorHeap)));
			return descriptorHeap;
		}()
	},
	m_DescriptorSize{ renderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(type) },
	m_HeapAllocationManager{ renderDevice, *this, 0, m_DescriptorHeap, 0, numDescriptorsInHeap },
	m_DynamicAllocationsManager{ renderDevice, *this, 1, m_DescriptorHeap, numDescriptorsInHeap, numDynamicDescriptors }
	{

	}

	GPUDescriptorHeap::~GPUDescriptorHeap()
	{

	}

	void GPUDescriptorHeap::Free(DescriptorHeapAllocation&& allocation)
	{
		struct StaleAllocation
		{
			DescriptorHeapAllocation Allocation;
			GPUDescriptorHeap* Heap;

			// clang-format off
			StaleAllocation(DescriptorHeapAllocation&& _Allocation, GPUDescriptorHeap& _Heap)noexcept :
				Allocation{ std::move(_Allocation) },
				Heap{ &_Heap }
			{
			}

			StaleAllocation(const StaleAllocation&) = delete;
			StaleAllocation& operator= (const StaleAllocation&) = delete;
			StaleAllocation& operator= (StaleAllocation&&) = delete;

			StaleAllocation(StaleAllocation&& rhs)noexcept :
				Allocation{ std::move(rhs.Allocation) },
				Heap{ rhs.Heap }
			{
				rhs.Heap = nullptr;
			}
			// clang-format on

			~StaleAllocation()
			{
				if (Heap != nullptr)
				{
					auto MgrId = Allocation.GetAllocationManagerId();
					assert((MgrId == 0 || MgrId == 1) && "Unexpected allocation manager ID");

					if (MgrId == 0)
					{
						Heap->m_HeapAllocationManager.FreeAllocation(std::move(Allocation));
					}
					else
					{
						Heap->m_DynamicAllocationsManager.FreeAllocation(std::move(Allocation));
					}
				}
			}
		};

		m_RenderDevice.SafeReleaseDeviceObject(StaleAllocation{ std::move(allocation), *this });
	}

	// ---------------------------- DynamicSuballocationsManager ------------------------
	DynamicSuballocationsManager::DynamicSuballocationsManager(GPUDescriptorHeap& parentGPUHeap,
		UINT32 dynamicChunkSize,
		std::string managerName) :
		m_ParentGPUHeap{ parentGPUHeap },
		m_DynamicChunkSize{ dynamicChunkSize },
		m_ManagerName{ managerName }
	{

	}

	DynamicSuballocationsManager::~DynamicSuballocationsManager()
	{
		assert(m_Chunks.empty() && m_CurrDescriptorCount == 0 && m_CurrChunkSize == 0 && "All dynamic suballocations must be released!");
	}

	// Release all allocated Chunks, these Chunks will be added to the release queue, and will eventually be recycled by ParentGPUHeap
	void DynamicSuballocationsManager::ReleaseAllocations()
	{
		for (auto& Allocation : m_Chunks)
		{
			m_ParentGPUHeap.Free(std::move(Allocation));
		}
		m_Chunks.clear();
		m_CurrDescriptorCount = 0;
		m_CurrChunkSize = 0;
	}

	// May waste chunck space, for example, the penultimate still has free memory, but it not enought to store this allocation
	// This will create a new Chunk, and all subsequent allocations will be performed in this new Chunk, 
	// thus wasting part of the free memory of the previous Chunk
	DescriptorHeapAllocation DynamicSuballocationsManager::Allocate(UINT32 count)
	{
		// If there is not a Chunk or the last Chunk dont have enougth space, allocate a Chunck large enought
		if (m_Chunks.empty() || m_CurrentOffsetInChunk + count > m_Chunks.back().GetNumHandles())
		{
			auto newChunkSize = std::max(m_DynamicChunkSize, count);
			auto newChunk = m_ParentGPUHeap.AllocateDynamic(newChunkSize);
			if (newChunk.IsNull())
			{
				LOG_ERROR("GPU Descriptor heap is full.");
				return DescriptorHeapAllocation();
			}

			m_Chunks.emplace_back(std::move(newChunk));
			m_CurrentOffsetInChunk = 0;

			m_CurrChunkSize += newChunkSize;
			m_PeakSuballocationsTotalSize = std::max(m_PeakSuballocationsTotalSize, m_CurrChunkSize);
		}

		auto& currentSuballocation = m_Chunks.back();

		auto managerID = currentSuballocation.GetAllocationManagerId();

		DescriptorHeapAllocation allocation(*this,
			currentSuballocation.GetDescriptorHeap(),
			currentSuballocation.GetCpuHandle(m_CurrentOffsetInChunk),
			currentSuballocation.GetGpuHandle(m_CurrentOffsetInChunk),
			count,
			static_cast<UINT16>(managerID));
		m_CurrentOffsetInChunk += count;
		m_CurrDescriptorCount += count;
		m_PeakDescriptorCount = std::max(m_PeakDescriptorCount, m_CurrDescriptorCount);

		return allocation;
	}
}