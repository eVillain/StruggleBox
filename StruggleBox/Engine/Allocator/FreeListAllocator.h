#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

// The FreeList allocator allows allocations of any size to be made
// (inside the available memory) and deallocations in any order.
// A linked-list of free blocks of memory is maintained (each free
// block contains information about its size and a pointer to the next free block).

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