#pragma once

#include "00_basicFile.h"

namespace zhang::sequence_containers
{
#ifdef __HAS_CPP20

	// 预定义一些用于 简写 和 标志识别 的宏
	#ifndef __zh_namespace

		#define __zh_namespace ::zhang::

	#endif // !__zh_namespace


	/* heap 算法 */
	namespace namespace_heap
	{
		// push_heap() -- 辅助函数
		template <__is_random_access_iterator RandomAccessIterator,
				  typename Distance,
				  typename T,
				  class Function	  = _RANGES	  less,
				  typename Projection = _STD identity>
		inline void __push_heap(RandomAccessIterator first,
								Distance			 holeIndex,
								Distance			 topIndex,
								T					 value,
								Function			 fun  = {},
								Projection			 proj = {})
		{
			Distance parent = (holeIndex - 1) / 2;

			while ((topIndex < holeIndex) && __invoke(fun, __invoke(proj, *(first + parent)), value))
			{
				*(first + holeIndex) = __invoke(proj, *(first + parent));
				holeIndex			 = parent;
				parent				 = (holeIndex - 1) / 2;
			}

			*(first + holeIndex) = value;
		}

		// push_heap() for 仿函数 标准版
		template <__is_random_access_iterator RandomAccessIterator,
				  class Function	  = _RANGES	   less,
				  typename Projection = _STD  identity>
		inline void
			push_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}, Projection proj = {})
		{
			fun = __check_fun(fun);

			using value_type = __value_type_for_iter<RandomAccessIterator>;
			using Distance	 = __difference_type_for_iter<RandomAccessIterator>;

			namespace_heap::__push_heap(first,
										__cove_type((last - first) - 1, Distance),
										__init_type(0, Distance),
										__cove_type(*(last - 1), value_type),
										fun,
										proj);
		}

		// push_heap() for 容器、仿函数 强化版
		template <__is_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		inline void push_heap(Range& con, Function fun = {}, Projection proj = {})
		{
			namespace_heap::push_heap(__begin_for_container(con), __end_for_container(con), fun, proj);
		}

		// pop_heap -- 辅助函数
		template <__is_random_access_iterator RandomAccessIterator,
				  typename Distance,
				  typename T,
				  class Function	  = _RANGES	  less,
				  typename Projection = _STD identity>
		inline void __adjust_heap(RandomAccessIterator first,
								  Distance			   holeIndex,
								  Distance			   len,
								  T					   value,
								  Function			   fun	= {},
								  Projection		   proj = {})
		{
			Distance topIndex { holeIndex };
			Distance secondChild { 2 * holeIndex + 2 };

			while (secondChild < len)
			{
				if (__invoke(fun, __invoke(proj, *(first + secondChild)), __invoke(proj, *(first + (secondChild - 1)))))
				{
					secondChild--;
				}

				*(first + holeIndex) = __invoke(proj, *(first + secondChild));
				holeIndex			 = secondChild;
				secondChild			 = 2 * (secondChild + 1);
			}

			if (secondChild == len)
			{
				*(first + holeIndex) = __invoke(proj, *(first + (secondChild - 1)));
			}

			namespace_heap::__push_heap(first, holeIndex, topIndex, value, fun, proj);
		}

		// pop_heap() -- 辅助函数
		template <__is_random_access_iterator RandomAccessIterator,
				  typename T,
				  class Function	  = _RANGES	  less,
				  typename Projection = _STD identity>
		inline void __pop_heap(RandomAccessIterator first,
							   RandomAccessIterator last,
							   RandomAccessIterator result,
							   T					value,
							   Function				fun	 = {},
							   Projection			proj = {})
		{
			using distance_type = __difference_type_for_iter<RandomAccessIterator>;

			*result = __invoke(proj, *first);

			namespace_heap::__adjust_heap(first,
										  __init_type(0, distance_type),
										  __cove_type(last - first, distance_type),
										  value,
										  fun,
										  proj);
		}

		// pop_heap() for 仿函数 标准版
		template <__is_random_access_iterator RandomAccessIterator,
				  class Function	  = _RANGES	   less,
				  typename Projection = _STD  identity>
		inline void
			pop_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}, Projection proj = {})
		{
			fun = __check_fun(fun);

			using value_type = __value_type_for_iter<RandomAccessIterator>;

			namespace_heap::__pop_heap(first, last - 1, last - 1, __cove_type(*(last - 1), value_type), fun, proj);
		}

		// pop_heap() for 容器、仿函数 强化版
		template <__is_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		inline void pop_heap(Range& con, Function fun = {}, Projection proj = {})
		{
			namespace_heap::pop_heap(__begin_for_container(con), __end_for_container(con), fun, proj);
		}

		// sort_heap() for 仿函数 标准版
		template <__is_random_access_iterator RandomAccessIterator,
				  class Function	  = _RANGES	   less,
				  typename Projection = _STD  identity>
		inline void
			sort_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}, Projection proj = {})
		{
			fun = __check_fun(fun);

			while (1 < (last - first))
			{
				namespace_heap::pop_heap(first, last--, fun, proj);
			}
		}

		// sort_heap() for 容器、仿函数 强化版
		template <__is_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		inline void sort_heap(Range& con, Function fun = {}, Projection proj = {})
		{
			namespace_heap ::sort_heap(__begin_for_container(con), __end_for_container(con), fun, proj);
		}

		// make_heap() for 仿函数 标准版
		template <__is_random_access_iterator RandomAccessIterator,
				  class Function	  = _RANGES	   less,
				  typename Projection = _STD  identity>
		inline void
			make_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}, Projection proj = {})
		{
			if ((last - first) < 2)
			{
				return;
			}

			fun = __check_fun(fun);

			using value_type	= __value_type_for_iter<RandomAccessIterator>;
			using distance_type = __difference_type_for_iter<RandomAccessIterator>;

			distance_type len	 = last - first;
			distance_type parent = (len - 2) / 2;

			while (true)
			{
				namespace_heap::__adjust_heap(first,
											  parent,
											  len,
											  __cove_type(*(first + parent), value_type),
											  fun,
											  proj);

				if (parent == 0)
				{
					return;
				}

				parent--;
			}
		}

		// make_heap() for 仿函数、容器 强化版
		template <__is_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		inline void make_heap(Range& con, Function fun = {}, Projection proj = {})
		{
			namespace_heap::make_heap(__begin_for_container(con), __end_for_container(con), fun, proj);
		}
	} // namespace namespace_heap

	// 对外接口
	using namespace_heap::make_heap;
	using namespace_heap::pop_heap;
	using namespace_heap::push_heap;
	using namespace_heap::sort_heap;


	#ifdef __zh_namespace
		#undef __zh_namespace
	#endif // __zh_namespace

#endif	   // __HAS_CPP20
} // namespace zhang::sequence_containers
