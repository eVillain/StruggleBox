#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

#include "Allocator.h"

// A Stack Allocator, like the name says, works like a stack.
// Along with the stack size, three pointers are maintained:
// - Pointer to the start of the stack.
// - Pointer to the top of the stack.
// - Pointer to the last allocation made. (This is optional in release builds)
// New allocations move the pointer up by the requested number of bytes plus the adjustment needed to align the address and store the allocation header.
// The allocation header provides the following information:
// - Adjustment used in this allocation
// - Pointer to the previous allocation.
// To deallocate memory the allocator checks if the address to the memory that you want to deallocate corresponds to the address of the last allocation made.
// If so the allocator accesses the allocation header so it also frees the memory used to align the allocation and store the allocation header,
// and it replaces the pointer to the last allocation made with the one in the allocation header.
// NOTE: Memory must be deallocated in inverse order it was allocated!
// So if you allocate object A and then object B you must free object B memory before you can free object A memory.

class StackAllocator : public Allocator
{
public:
	StackAllocator(size_t size, void* start);
	~StackAllocator();

	void* allocate(size_t size, uint8_t alignment = 4) override;
		
	void deallocate(void* p) override;

private:
	StackAllocator(const StackAllocator&); //Prevent copies because it might cause errors
	StackAllocator& operator=(const StackAllocator&);

	struct AllocationHeader
	{
		#if _DEBUG
		void* prev_address;
		#endif
		uint8_t adjustment;
	};

	#if _DEBUG
	void* _prev_position;
	#endif

	void*  _current_pos;
};

namespace allocator
{
	inline StackAllocator* newStackAllocator(size_t size, Allocator& allocator)
	{
		void* p = allocator.allocate(size+sizeof(StackAllocator), __alignof(StackAllocator));
		return new (p) StackAllocator(size, pointer_math::add(p, sizeof(StackAllocator)));
	}

	inline void deleteStackAllocator(StackAllocator& stackAllocator, Allocator& allocator)
	{
		stackAllocator.~StackAllocator();

		allocator.deallocate(&stackAllocator);
	}
};

#endif