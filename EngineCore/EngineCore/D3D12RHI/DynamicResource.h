#pragma once

// https://github.com/DiligentGraphics/DiligentCore/blob/master/Graphics/GraphicsEngineD3D12/include/D3D12DynamicHeap.hpp
// http://diligentgraphics.com/2016/04/20/implementing-dynamic-resources-with-direct3d12/

namespace RHI
{
	// Indicates a dynamic allocation, recording the ID3D12Resource to which it belongs, 
	//the size of the allocation, and the CPU and GPU addresses
	struct D3D12DynamicAllocation
	{
		D3D12DynamicAllocation() noexcept {}
		D3D12DynamicAllocation(ID3D12Resource* pBuff,
			UINT64                    _Offset,
			UINT64                    _Size,
			void* _CPUAddress,
			D3D12_GPU_VIRTUAL_ADDRESS _GPUAddress
		) noexcept :
			pBuffer{ pBuff },
			Offset{ _Offset },
			Size{ _Size },
			CPUAddress{ _CPUAddress },
			GPUAddress{ _GPUAddress }
		{}

		ID3D12Resource* pBuffer = nullptr;
		UINT64 Offset = 0;
		UINT64 Size = 0;
		void* CPUAddress = nullptr;                     // The CPU-writeable address
		D3D12_GPU_VIRTUAL_ADDRESS GPUAddress = 0;       // The GPU-visible address
	};

	// Represent a piece of memory in GPU. The constructor of DynamicPage will create a Buffer in the Upload heap of D3D.
	// The resources to be allocated are all sub-allocated in this page.
	class D3D12DynamicPage
	{
	public:
		D3D12DynamicPage(UINT64 Size);

		D3D12DynamicPage(D3D12DynamicPage&&) = default;

		D3D12DynamicPage(const D3D12DynamicPage&) = delete;
		D3D12DynamicPage& operator= (const D3D12DynamicPage&) = delete;
		D3D12DynamicPage& operator= (D3D12DynamicPage&&) = delete;

		void* GetCPUAddress(UINT64 offset)
		{
			assert(m_pd3d12Buffer != nullptr);
			assert(offset < GetSize());
			return reinterpret_cast<UINT8*>(m_CPUVirtualAddress) + offset;
		}

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(UINT64 Offset)
		{
			assert(m_pd3d12Buffer != nullptr);
			assert(Offset < GetSize());
			return m_GPUVirtualAddress + Offset;
		}

		ID3D12Resource* GetD3D12Buffer()
		{
			return m_pd3d12Buffer.Get();
		}

		UINT64 GetSize()
		{
			assert(m_pd3d12Buffer != nullptr);
			return m_pd3d12Buffer->GetDesc().Width;
		}

		bool IsValid() const { return m_pd3d12Buffer != nullptr; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pd3d12Buffer;
		void* m_CPUVirtualAddress = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS m_GPUVirtualAddress = 0;
	};

	// Manage all the memory used by dynamic resources, there is only one copy globally
	class DynamicResourceAllocator
	{
	public:
		DynamicResourceAllocator(UINT32 NumPagesToReserve, UINT64 PageSize);
		~DynamicResourceAllocator();

		DynamicResourceAllocator(const DynamicResourceAllocator&) = delete;
		DynamicResourceAllocator(DynamicResourceAllocator&&) = delete;
		DynamicResourceAllocator& operator= (const DynamicResourceAllocator&) = delete;
		DynamicResourceAllocator& operator= (DynamicResourceAllocator&&) = delete;

		void ReleasePages(std::vector<D3D12DynamicPage>& Pages);

		void Destroy();

		D3D12DynamicPage AllocatePage(UINT64 SizeInBytes);
	private:
		std::multimap<UINT64/*Page Size*/, D3D12DynamicPage, std::less<UINT64>> m_AvailablePages;
	};

	// Dynamic resource
	class DynamicResourceHeap
	{
	public:
		DynamicResourceHeap(DynamicResourceAllocator& DynamicMemAllocator, UINT64 PageSize): 
			m_GlobalDynamicAllocator{ DynamicMemAllocator },
			m_BasePageSize{ PageSize } {  }

		DynamicResourceHeap(const DynamicResourceHeap&) = delete;
		DynamicResourceHeap(DynamicResourceHeap&&) = delete;
		DynamicResourceHeap& operator= (const DynamicResourceHeap&) = delete;
		DynamicResourceHeap& operator= (DynamicResourceHeap&&) = delete;

		~DynamicResourceHeap();

		D3D12DynamicAllocation Allocate(UINT64 SizeInByte, UINT64 Alignment);

		void ReleaseAllocatedPages();

		static constexpr UINT64 InvalidOffset = static_cast<UINT64>(-1);
	private:
		DynamicResourceAllocator& m_GlobalDynamicAllocator;

		// Allocated smallest page size, if the size is not enougth, incresed by 2
		const UINT64 m_BasePageSize;

		std::vector<D3D12DynamicPage> m_AllocatedPages;

		// Offset allocated in current Page
		UINT64 m_CurrOffset = InvalidOffset;
		// The remaining capacity of the current Page
		UINT64 m_AvailableSize = 0;
		// allocated size
		UINT64 m_CurrAllocatedSize = 0;
		// used size
		UINT64 m_CurrUsedSize = 0;
		// Aligned size
		UINT64 m_CurrUsedAlignedSize = 0;
		UINT64 m_PeakAllocatedSize = 0;
		// peak
		UINT64 m_PeakUsedSize = 0;
		UINT64 m_PeakAlignedSize = 0;
	};
}