#pragma once

/* https://github.com/DiligentGraphics/DiligentCore/blob/14230a6655b012cd74622b123bae6d14d08ffad3/Graphics/GraphicsAccessories/interface/VariableSizeAllocationsManager.hpp
* Managing the blocks of memory requested. Use 2 ordered maps(1- keeps blocks sorted by their offset, 2- keeps blocks by their size)
* The elements of 2 maps reference each other.
* ---- 16 ----- | | ---- 32 ---- 48 -----| | -> Memory
*	offset = 8		offset = 32
*	size = 16       size = 24
*/
namespace RHI
{
	class VariableSizeAllocationsManager
	{
	private:
		struct FreeBlockInfo;
		// Free memory blocks sorted by offset
		using TFreeBlocksByOffsetMap = std::map<size_t, FreeBlockInfo, std::less<size_t>>;
		// / Free memory blocks sorted by size, multimap is used here, because the size of multiple memory blocks may be the same,
		//but the offset will not be the same
		using TFreeBlocksBySizeMap = std::multimap<size_t, TFreeBlocksByOffsetMap::iterator, std::less<size_t>>;

		struct FreeBlockInfo
		{
			// Block size (no reserved space for the size of allocation)
			size_t Size;

			// Iterator referenceing this block in the multimap sorted by block size
			TFreeBlocksBySizeMap::iterator orderBySizeIt;
			FreeBlockInfo(size_t size)
				:Size(size) {}
		};

	public:
		VariableSizeAllocationsManager(size_t maxSize)
			: m_MaxSize(maxSize),
			m_FreeSize(maxSize) 
		{
			
		}

		~VariableSizeAllocationsManager() {}
		
		// If the move constructor is declared as noexpect, the compiler will not generate exception handling code to improve efficiency, 
		//because usually the move constructor will not allocate memory and no exception will occur
		VariableSizeAllocationsManager(VariableSizeAllocationsManager&& rhs) noexcept :
			m_FreeBlocksByOffset{ std::move(rhs.m_FreeBlocksByOffset) },
			m_FreeBlocksBySize{ std::move(rhs.m_FreeBlocksBySize) },
			m_MaxSize{ rhs.m_MaxSize },
			m_FreeSize{ rhs.m_FreeSize },
			m_CurrAlignment{ rhs.m_CurrAlignment }
		{
			rhs.m_MaxSize = 0;
			rhs.m_FreeSize = 0;
			rhs.m_CurrAlignment = 0;
		}

		VariableSizeAllocationsManager& operator = (VariableSizeAllocationsManager&& rhs) = delete;
		VariableSizeAllocationsManager(const VariableSizeAllocationsManager&) = delete;
		VariableSizeAllocationsManager& operator = (const VariableSizeAllocationsManager&) = delete;

		// Offset returned by Allocate() may not be aligned, but the size of the allocation is sufficient to properly align it
		struct Allocation
		{
			Allocation(size_t offset, size_t size) :
				unalignedOffset{ offset },
				size{ size }
			{}

			Allocation() {}

			static constexpr size_t InvalidOffset = static_cast<size_t>(-1);

			static Allocation InvalidAllocation()
			{
				return Allocation{ InvalidOffset, 0 };
			}

			bool IsValid() const
			{
				return unalignedOffset != InvalidAllocation().unalignedOffset;
			}

			bool operator==(const Allocation& rhs) const
			{
				return unalignedOffset == rhs.unalignedOffset && size == rhs.size;
			}

			size_t size = 0;
			size_t unalignedOffset = InvalidOffset;
		};

		bool IsFull() const { return m_FreeSize == 0; }
		bool IsEmpty() const { return m_FreeSize == m_MaxSize; }
		size_t GetMaxSize() const { return m_MaxSize; }
		size_t GetFreeSize() const { return m_FreeSize; }
		size_t GetUsedSize() const { return m_MaxSize - m_FreeSize; }

		size_t GetFreeBlocksNum() const
		{
			return m_FreeBlocksByOffset.size();
		}

	private:

		void addNewBlock(size_t offset, size_t size)
		{
			auto newBlockIt = m_FreeBlocksByOffset.emplace(offset, size);
			auto orderIt = m_FreeBlocksBySize.emplace(size, newBlockIt.first);
			newBlockIt.first->second.orderBySizeIt = orderIt;
		}

		void ResetCurrAlignment()
		{
			for (m_CurrAlignment = 1; m_CurrAlignment * 2 <= m_MaxSize; m_CurrAlignment *= 2)
			{
			}
		}

		TFreeBlocksByOffsetMap m_FreeBlocksByOffset;
		TFreeBlocksBySizeMap m_FreeBlocksBySize;

		size_t m_MaxSize = 0;
		size_t m_FreeSize = 0;
		size_t m_CurrAlignment = 0;
	};
}