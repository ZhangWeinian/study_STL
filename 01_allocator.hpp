#ifndef __ZHALLOC__
#define __ZHALLOC__

#pragma once

#include <algorithm>
#include <climits>	// for UINT_MAX
#include <cstddef>	// for ptrdiff_t,size_t
#include <cstdlib>	// for exit()
#include <iostream> // for cerr
#include <new>		// for placement new

/* for myself fun */
#include "./00_basicFile.hpp"

namespace zhang
{
	template <typename t>
	inline t* _allocate(ptrdiff_t size, t*)
	{
		set_new_handler(0);

		auto tmp = _cove_type(::operator new(_cove_type(size * sizeof(t), size_t)), t*);

		if (tmp == nullptr)
		{
			cerr << "out of memory!" << endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			return tmp;
		}
	}

	template <typename t>
	inline void _deallocate(t* buffer)
	{
		::operator delete(buffer);
	}

	template <typename t1, typename t2>
	inline void _construct(t1* p, const t2& value)
	{
		new (p) t1(value); // 新位置。调用T1的ctor
	}

	template <typename t>
	inline void _destory(t* ptr)
	{
		ptr->~t();
	}

	template <typename t>
	class allocator
	{
	public:

		using value_type = t;
		using pointer = t*;
		using const_pointer = const t*;
		using reference = t&;
		using const_reference = const t&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

		// U 类型的重新绑定分配器
		template <typename u>
		struct rebind
		{
			using other = allocator<u>;
		};

		// 用于位置的提示
		pointer allocate(size_type n, const void* hint = nullptr)
		{
			return _allocate(
				_cove_type(n, difference_type),
				_cove_type(hint,
						   pointer)); // p_46：书中未使用参数 hint 而使用了 _cove_type(0, pointer) ，窃以为系刊误
		}

		void deallocate(pointer p, size_type n)
		{
			_deallocate(p);
		}

		void construct(pointer p, const_reference value)
		{
			_construct(p, value);
		}

		void destory(pointer p)
		{
			_destory(p);
		}

		pointer address(reference x)
		{
			return _cove_type(&x, pointer);
		}

		const_pointer const_address(const_reference x)
		{
			return _cove_type(&x, const_pointer);
		}

		size_type max_size(void) const
		{
			return _cove_type(UINT_MAX / sizeof(t), size_type);
		}
	};
} // namespace zhang


#endif // !__ZHALLOC__
