#pragma once

#include "00_basicFile.h"

namespace zhang::allocator
{
	// 预定义一些用于 简写 和 标志识别 的宏
#ifndef __ZH_NAMESPACE__

	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#define __ZH_NAMESPACE__ ::zhang::
	#define __ZH_ITER__		 ::zhang::iterator::namespace_iterator::

#endif // !__ZH_NAMESPACE__


	// 此处实现构造和析构的基本工具：construct() 和 destory()
	namespace namespace_de_construction
	{
		/* function construct() */
		template <typename T1, typename T2>
		inline void construct(T1* p, const T2& value)
		{
			new (p) T1(value);
		}

		/*-----------------------------------------------------------------------------------------*/



		// 以下是 destory()

		/* function destoey() 重载 3 */
		inline void destory(char*, char*)
		{
			// do nothing
		}

		/* function destoey() 重载 4 */
		inline void destory(wchar_t*, wchar_t*)
		{
			// do nothing
		}

		/* function destoey() 重载 2 */
		template <typename ForwardIterator> // 接受两个迭代器，找出元素型别从而采取最佳措施
		inline void destory(ForwardIterator first, ForwardIterator last)
		{
			namespace_de_construction::__destory(first, last, __ZH_ITER__ value_type(first));
		}

		/* function __destory_aux() 重载 2 -- 辅助函数 */
		template <typename ForwardIterator> // 如果元素的 value_type 有 trivial destructor
		inline void __destory_aux(ForwardIterator, ForwardIterator, __ZH_ITER__ __true_type)
		{
			// do nothing
		}

		/* function __destory_aux() 重载 1 -- 辅助函数 */
		template <typename ForwardIterator> // 如果元素的 value_type 有 non-trivial destructor
		inline void __destory_aux(ForwardIterator first, ForwardIterator last, __ZH_ITER__ __false_type)
		{
			for (; first != last; ++first)
			{
				namespace_de_construction::destory(&*first);
			}
		}

		/* function __destory() 重载 2 -- 辅助函数 */
		template <typename ForwardIterator, typename T>
		inline void __destory(ForwardIterator first, ForwardIterator last, T*)
		{
			using teivial_destructor = typename __ZH_ITER__ __type_traits<T>::has_trivial_destructor;

			namespace_de_construction::__destory_aux(first, last, teivial_destructor());
		}

		/* function destory() 重载 1 */
		template <typename T> // 接受一个指针
		inline void destory(T* pointer)
		{
			pointer->~T();
		}

	} // namespace namespace_de_construction

	// 此处实现 SGI allocator
	namespace namespace_alloc
	{
		// 这是一个外包装，使其接口符合规范，不论是第一级配置器还是第二级配置器
		template <typename T, typename Alloc>
		class simple_alloc
		{
			static T* allocate(size_t n)
			{
				return 0 == n ? nullptr : _cove_type(Alloc::allocate(sizeof(T)), T*);
			}

			static* allocate(void)
			{
				return _cove_type(Alloc::allocate(sizeof(T)), T*);
			}

			static void deallocate(T* p, size_t n)
			{
				if (0 != n)
				{
					Alloc::deallocate(p, n * sizeof(T));
				}
			}

			static void deallocate(T* p)
			{
				Alloc::deallocate(p, sizeof(T));
			}
		};
	} // namespace namespace_alloc

	// 对外统一接口
	using namespace_alloc::simple_alloc;
	using namespace_de_construction::construct;
	using namespace_de_construction::destory;

#ifdef __ZH_NAMESPACE__
	#undef __ZH_NAMESPACE__
	#undef __ZH_ITER__
#endif // __ZH_NAMESPACE__

} // namespace zhang::allocator
