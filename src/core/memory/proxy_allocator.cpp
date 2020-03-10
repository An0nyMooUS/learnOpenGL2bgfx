/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/memory/proxy_allocator.h"

namespace crown
{
ProxyAllocator::ProxyAllocator(Allocator& allocator, const char* name)
	: _allocator(allocator)
	, _name(name)
{
	CE_ASSERT(name != NULL, "Name must be != NULL");
}

void* ProxyAllocator::allocate(u32 size, u32 align)
{
	void* p = _allocator.allocate(size, align);
	return p;
}

void ProxyAllocator::deallocate(void* data)
{
	_allocator.deallocate(data);
}

const char* ProxyAllocator::name() const
{
	return _name;
}

} // namespace crown
