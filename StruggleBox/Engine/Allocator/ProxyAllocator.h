#ifndef PROXYALLOCATOR_H
#define PROXYALLOCATOR_H

#include "Allocator.h"

// A Proxy Allocator is a special kind of allocator.
// It is just used to help with memory leak and subsystem memory usage tracking.
// It will simply redirect all allocations / deallocations to the allocator passed as argument in the constructor,
// while keeping track of how many allocations it made and how much memory it is "using".
// Example: Two subsystems use the same allocator A.
// If you want to show in the debugging user interface how much memory each subsystem is using you create a proxy allocator,
// that redirects all allocations / deallocations to A in each subsystem and track their memory usage.
// It will also help in memory leak tracking because the assert in the proxy allocator destructor of the subsystem that is leaking memory will fail.

class ProxyAllocator : public Allocator
{
public:
	ProxyAllocator(Allocator& allocator);
	~ProxyAllocator();

	void* allocate(size_t size, uint8_t alignment = 4) override;
		
	void deallocate(void* p) override;

private:
	ProxyAllocator(const ProxyAllocator&); //Prevent copies because it might cause errors
	ProxyAllocator& operator=(const ProxyAllocator&);

	Allocator& _allocator;
};

namespace allocator
{
	inline ProxyAllocator* newProxyAllocator(Allocator& allocator)
	{
		void* p = allocator.allocate(sizeof(ProxyAllocator), __alignof(ProxyAllocator));
		return new (p) ProxyAllocator(allocator);
	}

	inline void deleteProxyAllocator(ProxyAllocator& proxyAllocator, Allocator& allocator)
	{
		proxyAllocator.~ProxyAllocator();

		allocator.deallocate(&proxyAllocator);
	}
};

#endif