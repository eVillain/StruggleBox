#ifndef ARENA_OPERATORS_H
#define ARENA_OPERATORS_H

// Macros to use the allocators with placement new operators
// Entirely based on the awesome Molecule Engine blog posts:
// https://blog.molecular-matters.com/2011/07/05/memory-system-part-1/
// https://blog.molecular-matters.com/2011/07/07/memory-system-part-2/

// TODO: Get file and line where allocation occurs for tracking allocations
//#define ME_NEW(type, arena)    new (arena.allocate(sizeof(type), __FILE__, __LINE__)) type
//#define ME_NEW_ARRAY(type, arena)    NewArray<TypeAndCount<type>::Type>(arena, TypeAndCount<type>::Count, __FILE__, __LINE__)
#define CUSTOM_NEW(type, arena)    new (arena.allocate(sizeof(type))) type
#define CUSTOM_DELETE(object, arena)    Delete(object, arena)
//#define CUSTOM_NEW_ARRAY(type, arena)    NewArray<TypeAndCount<type>::Type>(arena, TypeAndCount<type>::Count)
#define CUSTOM_NEW_ARRAY(type, count, arena)    NewArray<type>(arena, count)
#define CUSTOM_DELETE_ARRAY(object, arena)    DeleteArray(object, arena)

template <typename T, class ARENA>
void Delete(T* object, ARENA& arena)
{
	// call the destructor first...
	object->~T();

	// ...and free the associated memory
	arena.deallocate(object);
}

template <class T>
struct TypeAndCount
{
};

template <class T, size_t N>
struct TypeAndCount<T[N]>
{
	typedef T Type;
	static const size_t Count = N;
};

template <typename T, class ARENA>
T* NewArray(ARENA& arena, size_t N/*, const char* file, int line*/)
{
	union
	{
		void* as_void;
		size_t* as_size_t;
		T* as_T;
	};

	as_void = arena.allocate(sizeof(T)*N + sizeof(size_t)/*, file, line*/);

	// store number of instances in first size_t bytes
	*as_size_t++ = N;

	// construct instances using placement new
	const T* const onePastLast = as_T + N;
	while (as_T < onePastLast)
		new (as_T++) T;

	// hand user the pointer to the first instance
	return (as_T - N);
}

template <typename T, class ARENA>
void DeleteArray(T* ptr, ARENA& arena)
{
	union
	{
		size_t* as_size_t;
		T* as_T;
	};

	// user pointer points to first instance...
	as_T = ptr;

	// ...so go back size_t bytes and grab number of instances
	const size_t N = as_size_t[-1];

	// call instances' destructor in reverse order
	for (size_t i = N; i>0; --i)
		as_T[i - 1].~T();

	arena.deallocate(as_size_t - 1);
}

#endif
