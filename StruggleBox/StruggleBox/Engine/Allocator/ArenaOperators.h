#pragma once

// Macros to use the allocators with placement new operators
// Entirely based on the awesome Molecule Engine blog posts:
// https://blog.molecular-matters.com/2011/07/05/memory-system-part-1/
// https://blog.molecular-matters.com/2011/07/07/memory-system-part-2/

#ifdef _DEBUG
#define CUSTOM_NEW(type, arena)    new (arena.allocateDebug(sizeof(type), __FILE__, __LINE__)) type
#define CUSTOM_DELETE(object, arena)    Delete(object, arena, __FILE__, __LINE__)
//#define CUSTOM_NEW_ARRAY(type, arena)    NewArray<TypeAndCount<type>::Type>(arena, TypeAndCount<type>::Count, __FILE__, __LINE__)
#define CUSTOM_NEW_ARRAY(type, count, arena)    NewArray<type>(arena, count, __FILE__, __LINE__)
#define CUSTOM_DELETE_ARRAY(object, arena)    DeleteArray(object, arena, __FILE__, __LINE__)
#else
#define CUSTOM_NEW(type, arena)    new (arena.allocate(sizeof(type))) type
#define CUSTOM_DELETE(object, arena)    Delete(object, arena)
//#define CUSTOM_NEW_ARRAY(type, arena)    NewArray<TypeAndCount<type>::Type>(arena, TypeAndCount<type>::Count)
#define CUSTOM_NEW_ARRAY(type, count, arena)    NewArray<type>(arena, count)
#define CUSTOM_DELETE_ARRAY(object, arena)    DeleteArray(object, arena)
#endif


#ifdef _DEBUG
template <typename T, class ARENA>
void Delete(T* object, ARENA& arena, const char* file, int line)
{
	// call the destructor first...
	object->~T();

	// ...and free the associated memory
	arena.deallocateDebug(object, file, line);
}
#else
template <typename T, class ARENA>
void Delete(T* object, ARENA& arena)
{
	// call the destructor first...
	object->~T();

	// ...and free the associated memory
	arena.deallocate(object);
}
#endif
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

#ifdef _DEBUG
template <typename T, class ARENA>
T* NewArray(ARENA& arena, size_t N, const char* file, int line)
#else
template <typename T, class ARENA>
T* NewArray(ARENA& arena, size_t N/*, const char* file, int line*/)
#endif
{
	union
	{
		void* as_void;
		size_t* as_size_t;
		T* as_T;
	};
#ifdef _DEBUG
	as_void = arena.allocateDebug(sizeof(T) * N + sizeof(size_t), file, line);
#else
	as_void = arena.allocate(sizeof(T)*N + sizeof(size_t));
#endif
	// store number of instances in first size_t bytes
	*as_size_t++ = N;

	// construct instances using placement new
	const T* const onePastLast = as_T + N;
	while (as_T < onePastLast)
		new (as_T++) T;

	// hand user the pointer to the first instance
	return (as_T - N);
}

#ifdef _DEBUG
template <typename T, class ARENA>
void DeleteArray(T* ptr, ARENA& arena, const char* file, int line)
#else
template <typename T, class ARENA>
void DeleteArray(T* ptr, ARENA& arena)
#endif
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

#ifdef _DEBUG
	arena.deallocateDebug(as_size_t - 1, file, line);
#else
	arena.deallocate(as_size_t - 1);
#endif
}
