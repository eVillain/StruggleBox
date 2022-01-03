#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

// The FreeList allocator allows allocations of any size to be made (inside the available memory) and deallocations in any order.
// A linked - list of free blocks of memory is maintained (each free block contains information about its size and a pointer to the next free block).
// The allocator tries to find a free block large enough for the allocation to fit, if it finds multiple free blocks that meet the requeriments,
// there's 3 simple ways to decide which free block to choose:
// - First-fit - Use the first.
// - Best-fit - Use the smallest.
// - Worst-fit - Use the largest.
// The best-fit method will in most cases cause less fragmentation than the other 2 methods.
// The allocator keeps the free blocks orderer by the start position.
// When an allocation is freed the allocator finds the right position in the free blocks list and tries to merge it with the adjacent blocks.

#include "Allocator.h"

class FreeListAllocator : public Allocator
{
public:
	FreeListAllocator(size_t size, void* start);
	~FreeListAllocator();

	void* allocate(size_t size, uint8_t alignment = 4) override;
	void deallocate(void* p) override;
private:

	struct AllocationHeader
	{
		size_t size;
		uint8_t     adjustment;
	};

	struct FreeBlock
	{
		size_t     size;
		FreeBlock* next;
	};

	FreeListAllocator(const FreeListAllocator&); //Prevent copies because it might cause errors
	FreeListAllocator& operator=(const FreeListAllocator&);

	FreeBlock* _free_blocks;
};

namespace allocator
{
	inline FreeListAllocator* newFreeListAllocator(size_t size, Allocator& allocator)
	{
		void* p = allocator.allocate(size+sizeof(FreeListAllocator), __alignof(FreeListAllocator));
		return new (p) FreeListAllocator(size, pointer_math::add(p, sizeof(FreeListAllocator)));
	}

	inline void deleteFreeListAllocator(FreeListAllocator& freeListAllocator, Allocator& allocator)
	{
		freeListAllocator.~FreeListAllocator();

		allocator.deallocate(&freeListAllocator);
	}
};

#endif