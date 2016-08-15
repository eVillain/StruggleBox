#include "StackAllocator.h"

StackAllocator::StackAllocator(size_t size, void* start) 
	: Allocator(size, start), _current_pos(start)
{
	assert(size > 0);

	#if _DEBUG
	_prev_position    = nullptr;
	#endif
}

StackAllocator::~StackAllocator()
{
	#if _DEBUG
	_prev_position      = nullptr;
	#endif

	_current_pos   = nullptr;
}

void* StackAllocator::allocate(size_t size, uint8_t alignment)
{
	assert(size != 0);

	uint8_t adjustment = pointer_math::alignForwardAdjustmentWithHeader(_current_pos, alignment, sizeof(AllocationHeader));

	if(_used_memory + adjustment + size > _size)
		return nullptr;

	void* aligned_address = pointer_math::add(_current_pos, adjustment);

	//Add Allocation Header
	AllocationHeader* header = (AllocationHeader*)(pointer_math::subtract(aligned_address, sizeof(AllocationHeader)));

	header->adjustment   = adjustment;

	#if _DEBUG
	header->prev_address = _prev_position;

	_prev_position        = aligned_address;
	#endif

	_current_pos = pointer_math::add(aligned_address, size);

	_used_memory += size + adjustment;
	_num_allocations++;

	return aligned_address;
}

void StackAllocator::deallocate(void* p)
{
	assert( p == _prev_position );

	//Access the AllocationHeader in the bytes before p
	AllocationHeader* header = (AllocationHeader*)(pointer_math::subtract(p, sizeof(AllocationHeader)));

	_used_memory -= (uintptr_t)_current_pos - (uintptr_t)p + header->adjustment;

	_current_pos = pointer_math::subtract(p, header->adjustment);

	#if _DEBUG
	_prev_position = header->prev_address;
	#endif

	_num_allocations--;
}