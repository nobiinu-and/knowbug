// allocator to use memory buffer of hsp-runtime

#ifndef IG_HSP_ALLOCATOR_H
#define IG_HSP_ALLOCATOR_H

#include <cassert>
#include <numeric>
#include "hsp3plugin.h"

// �C���^�[�t�F�[�X�ɂȂ��̂ŁAhspexpand �͎g�p����Ȃ��B
// �v���O�C�����o�^���ꂽ��łȂ��Ǝg�p�ł��Ȃ��B

namespace hpimod
{

template<typename T>
class HspAllocator
{
public:
	using value_type = T;

	using difference_type = ptrdiff_t;
	using pointer = value_type*;
	using const_pointer = value_type const*;
	using reference = value_type&;
	using const_reference = value_type const&;

	HspAllocator() = default;
	~HspAllocator() = default;

	template <class U>
	HspAllocator(HspAllocator<U> const&) { }

	pointer allocate(size_t cntElements)
	{
		assert(exinfo && hspmalloc);
		return reinterpret_cast<pointer>(hspmalloc(cntElements * sizeof(T)));
	}
#if 0
	pointer allocate(size_t n, void const* hint)
	{
		return allocate(n, nullptr);
	}
#endif

	void deallocate(pointer p, size_t cntElements)
	{
		if ( p ) hspfree(p);
		return;
	}
};

// ��r���Z�q
template<typename T, typename U>
bool operator==(HspAllocator<T> const&, HspAllocator<U> const&) { return true; }

template<typename T, typename U>
bool operator!=(HspAllocator<T> const&, HspAllocator<U> const&) { return false; }

}

#endif
