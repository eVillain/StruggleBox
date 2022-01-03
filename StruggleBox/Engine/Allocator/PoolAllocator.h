#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H

#include "Allocator.h"

// This allocator only allows allocations of a fixed size and alignment to be made,
// this results in both fast allocations and deallocations to be made.
// Like the FreeList allocator, a linked-list of free blocks is maintaied but since all blocks are the same size each free block only needs to store a pointer to the next one.
// Another advantage of Pool allactors is no need to align each allocation,
// since all allocations have the same size / alignment only the first block has to be aligned.
// This results in a almost non-existant memory overhead.
// The block size of the Pool Allocator must be larger than sizeof(void*) because when blocks are free they store a pointer to the next free block in the list.
// The allocator simply returns the first free block and updates the linked list.
// During allocations the allocator simply returns the first free block and updates the linked list.
// During deallocations the allocator simply adds the deallocated block to the free blocks linked list.

class PoolAllocator : public Allocator
{
public:
	PoolAllocator(size_t objectSize, uint8_t objectAlignment, size_t size, void* mem);
	~PoolAllocator();

	void* allocate(size_t size, uint8_t alignment = 4) override;
		
	void deallocate(void* p) override;

private:
	PoolAllocator(const PoolAllocator&); //Prevent copies because it might cause errors
	PoolAllocator& operator=(const PoolAllocator&);

	size_t     _objectSize;
	uint8_t         _objectAlignment;

	void**     _free_list;
};

namespace allocator
{
	inline PoolAllocator* newPoolAllocator(size_t objectSize, uint8_t objectAlignment, size_t size, Allocator& allocator)
	{
		void* p = allocator.allocate(size+sizeof(PoolAllocator), __alignof(PoolAllocator));
		return new (p) PoolAllocator(objectSize, objectAlignment, size, pointer_math::add(p, sizeof(PoolAllocator)));
	}

	inline void deletePoolAllocator(PoolAllocator& poolAllocator, Allocator& allocator)
	{
		poolAllocator.~PoolAllocator();

		allocator.deallocate(&poolAllocator);
	}
};

#endif