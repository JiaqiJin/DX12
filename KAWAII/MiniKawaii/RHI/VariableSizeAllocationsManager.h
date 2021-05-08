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
			AddNewBlock(0, maxSize);
			ResetCurrAlignment();
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

		Allocation Allocate(size_t size, size_t alignment)
		{
			assert(size > 0);
			assert(IsPowerOfTwoD(alignment));

			size = Align(size, alignment);
			if (m_FreeSize < size)
				return Allocation::InvalidAllocation();

			auto alignmentReserve = (alignment > m_CurrAlignment) ? alignment - m_CurrAlignment : 0;

			// find the smaller block that is large enougth to emcompass the requested size
			auto smallestBlockItIt = m_FreeBlocksBySize.lower_bound(size + alignmentReserve);
			if (smallestBlockItIt == m_FreeBlocksBySize.end())
				return Allocation::InvalidAllocation();

			auto smallestBlockIt = smallestBlockItIt->second;
			assert((size + alignmentReserve) <= smallestBlockIt->second.Size);

			//     SmallestBlockIt.Offset
			//        |                                  |
			//        |<------SmallestBlockIt.Size------>|
			//        |<------Size------>|<---NewSize--->|
			//        |                  |
			//      Offset              NewOffset
			
			size_t offset = smallestBlockIt->first;
			assert(offset % m_CurrAlignment == 0);
			size_t alignedOffset = Align(offset, alignment);
			size_t adjustedSize = size + (alignedOffset - offset);
			assert(adjustedSize <= size + alignmentReserve);
			// the offset of new block will be given adding requested size to the original offset
			size_t newOffset = offset + adjustedSize;
			size_t newSize = smallestBlockIt->second.Size - adjustedSize;

			// updaing the maps.
			assert(smallestBlockItIt == smallestBlockIt->second.orderBySizeIt);
			m_FreeBlocksBySize.erase(smallestBlockItIt);
			m_FreeBlocksByOffset.erase(smallestBlockIt);

			if (newSize > 0)
				AddNewBlock(newOffset, newSize);

			m_FreeSize -= adjustedSize;

			if ((size & (m_CurrAlignment - 1)) != 0)
			{
				if (IsPowerOfTwoD(size))
				{
					assert(size >= alignment && size < m_CurrAlignment);
					m_CurrAlignment = size;
				}
				else
				{
					m_CurrAlignment = std::min(m_CurrAlignment, alignment);
				}
			}

			return Allocation(offset, adjustedSize);
		}

		void Free(Allocation&& allocation)
		{
			Free(allocation.unalignedOffset, allocation.size);
			allocation = Allocation{};
		}

		void Free(size_t offset, size_t size)
		{
			// Use the map sorted by Offset to find a free memory block behind the memory block to be released
			auto nextBlockIt = m_FreeBlocksByOffset.upper_bound(offset);

			// The memory block to be released cannot overlap with the next free memory block
			assert(nextBlockIt == m_FreeBlocksByOffset.end() || offset + size <= nextBlockIt->first);

			// The previous free memory block of the memory block to be released
			auto previousBlockIt = nextBlockIt;
			if (previousBlockIt != m_FreeBlocksByOffset.begin())
			{
				--previousBlockIt;
				// The memory block to be released cannot overlap with the previous free memory block
				assert(offset >= previousBlockIt->first + previousBlockIt->second.Size);
			}
			else
			{
				// The memory block to be released is the first memory block
				previousBlockIt = m_FreeBlocksByOffset.end();
			}

			// 4 cases
			size_t newSize, newOffset;
			// The memory block to be released is adjacent to the previous free memory block
			if ((previousBlockIt != m_FreeBlocksByOffset.end()) && (offset == previousBlockIt->first + previousBlockIt->second.Size))
			{
				//  PrevBlock.Offset             Offset
				//       |                          |
				//       |<-----PrevBlock.Size----->|<------Size-------->|
				//

				newSize = previousBlockIt->second.Size + size;
				newOffset = previousBlockIt->first;

				// The memory block to be released and the next free memory block are also adjacent, "merge" these three memory blocks
				if ((nextBlockIt != m_FreeBlocksByOffset.end()) && (offset + size == nextBlockIt->first))
				{
					//   PrevBlock.Offset           Offset            NextBlock.Offset
					//     |                          |                    |
					//     |<-----PrevBlock.Size----->|<------Size-------->|<-----NextBlock.Size----->|
					//

					newSize += nextBlockIt->second.Size;

					// Release the map of OrderBySize first, otherwise the iterator of OrderBySize will fail
					m_FreeBlocksBySize.erase(previousBlockIt->second.orderBySizeIt);
					m_FreeBlocksBySize.erase(nextBlockIt->second.orderBySizeIt);

					++nextBlockIt;
					m_FreeBlocksByOffset.erase(previousBlockIt, nextBlockIt);
				}
				// The memory block to be released is not adjacent to the next free memory block, but merges with the previous memory block
				else
				{
					//   PrevBlock.Offset           Offset                     NextBlock.Offset
					//     |                          |                             |
					//     |<-----PrevBlock.Size----->|<------Size-------->| ~ ~ ~  |<-----NextBlock.Size----->|
					//

					// 先释放OrderBySize的map，不然OrderBySize的迭代器会失效
					m_FreeBlocksBySize.erase(previousBlockIt->second.orderBySizeIt);
					m_FreeBlocksByOffset.erase(previousBlockIt);
				}
			}
			// The memory block to be released is not adjacent to the previous free memory block, 
			// but adjacent to the next free memory block, and merged with the next free memory block
			else if ((nextBlockIt != m_FreeBlocksByOffset.end()) && (offset + size == nextBlockIt->first))
			{
				//   PrevBlock.Offset                   Offset            NextBlock.Offset
				//     |                                  |                    |
				//     |<-----PrevBlock.Size----->| ~ ~ ~ |<------Size-------->|<-----NextBlock.Size----->|
				//
				newSize = size + nextBlockIt->second.Size;
				newOffset = offset;

				m_FreeBlocksBySize.erase(nextBlockIt->second.orderBySizeIt);
				m_FreeBlocksByOffset.erase(nextBlockIt);
			}
			// The memory block to be released is not adjacent to the two free memory blocks, only a new free memory block is inserted
			else
			{
				newSize = size;
				newOffset = offset;
			}

			AddNewBlock(newOffset, newSize);

			m_FreeSize += size;

			if (IsEmpty())
			{
				assert(GetFreeBlocksNum() == 1);
				ResetCurrAlignment();
			}
		}

		void Extend(size_t extraSize)
		{
			size_t newBlockOffset = m_MaxSize;
			size_t newBlockSize = extraSize;

			//If the last free memory block is at the end, merge with the last free memory block
			if (!m_FreeBlocksByOffset.empty())
			{
				auto lastBlockIt = m_FreeBlocksByOffset.end();
				--lastBlockIt;

				const auto lastBlockOffset = lastBlockIt->first;
				const auto lastBlockSize = lastBlockIt->second.Size;
				if (lastBlockOffset + lastBlockSize == m_MaxSize)
				{
					newBlockOffset = lastBlockOffset;
					newBlockSize += lastBlockSize;

					m_FreeBlocksBySize.erase(lastBlockIt->second.orderBySizeIt);
					m_FreeBlocksByOffset.erase(lastBlockIt);
				}
			}

			AddNewBlock(newBlockOffset, newBlockSize);

			m_MaxSize += extraSize;
			m_FreeSize += extraSize;
		}

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

		void AddNewBlock(size_t offset, size_t size)
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