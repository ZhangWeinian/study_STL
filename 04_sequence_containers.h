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
		template <__is_iterator_or_c_pointer RandomAccessIterator, typename Distance, typename T, typename Function>
		inline void
			__push_heap(RandomAccessIterator first, Distance holeIndex, Distance topIndex, T value, Function fun)
		{
			Distance parent = (holeIndex - 1) / 2;

			while ((topIndex < holeIndex) && fun(*(first + parent), value))
			{
				*(first + holeIndex) = *(first + parent);
				holeIndex			 = parent;
				parent				 = (holeIndex - 1) / 2;
			}

			*(first + holeIndex) = value;
		}

		// push_heap() for 仿函数 强化版
		template <__is_iterator_or_c_pointer RandomAccessIterator, typename Function>
		inline void push_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun)
		{
			fun = __check_fun(fun);

			using value_type = __value_type_for_iter<RandomAccessIterator>;
			using Distance	 = __difference_type_for_iter<RandomAccessIterator>;

			namespace_heap::__push_heap(first,
										__cove_type((last - first) - 1, Distance),
										__init_type(0, Distance),
										__cove_type(*(last - 1), value_type),
										fun);
		}

		// push_heap() 标准版
		template <__is_iterator_or_c_pointer RandomAccessIterator>
		inline void push_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			namespace_heap::push_heap(first, last, _RANGES less {});
		}

		// push_heap() for 容器、仿函数 强化版
		template <__is_container_or_c_array Container, typename Function>
		inline void push_heap(Container& con, Function fun)
		{
			namespace_heap::push_heap(__begin_for_container(con), __end_for_container(con), fun);
		}

		// push_heap() for 容器 强化版
		template <__is_container_or_c_array Container>
		inline void push_heap(Container& con)
		{
			namespace_heap::push_heap(__begin_for_container(con), __end_for_container(con), _RANGES less {});
		}

		// pop_heap -- 辅助函数
		template <__is_iterator_or_c_pointer RandomAccessIterator, typename Distance, typename T, typename Function>
		inline void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value, Function fun)
		{
			Distance topIndex	 = holeIndex;
			Distance secondChild = 2 * holeIndex + 2;

			while (secondChild < len)
			{
				if (fun(*(first + secondChild), *(first + (secondChild - 1))))
				{
					secondChild--;
				}

				*(first + holeIndex) = *(first + secondChild);
				holeIndex			 = secondChild;
				secondChild			 = 2 * (secondChild + 1);
			}

			if (secondChild == len)
			{
				*(first + holeIndex) = *(first + (secondChild - 1));
			}

			namespace_heap::__push_heap(first, holeIndex, topIndex, value, fun);
		}

		// pop_heap() -- 辅助函数
		template <__is_iterator_or_c_pointer RandomAccessIterator, typename T, typename Function>
		inline void __pop_heap(RandomAccessIterator first,
							   RandomAccessIterator last,
							   RandomAccessIterator result,
							   T					value,
							   Function				fun)
		{
			using distance_type = __difference_type_for_iter<RandomAccessIterator>;

			*result = *first;

			namespace_heap::__adjust_heap(first,
										  __init_type(0, distance_type),
										  __cove_type(last - first, distance_type),
										  value,
										  fun);
		}

		// pop_heap() for 仿函数 强化版
		template <__is_iterator_or_c_pointer RandomAccessIterator, typename Function>
		inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun)
		{
			fun = __check_fun(fun);

			using value_type = __value_type_for_iter<RandomAccessIterator>;

			namespace_heap::__pop_heap(first, last - 1, last - 1, __cove_type(*(last - 1), value_type), fun);
		}

		// pop_heap() 标准版
		template <__is_iterator_or_c_pointer RandomAccessIterator>
		inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			using value_type = __value_type_for_iter<RandomAccessIterator>;

			namespace_heap::__pop_heap(first,
									   last - 1,
									   last - 1,
									   __cove_type(*(last - 1), value_type),
									   _RANGES less {});
		}

		// pop_heap() for 容器、仿函数 强化版
		template <__is_container_or_c_array Container, typename Function>
		inline void pop_heap(Container& con, Function fun)
		{
			namespace_heap::pop_heap(__begin_for_container(con), __end_for_container(con), fun);
		}

		// pop_heap() for 容器 强化版
		template <__is_container_or_c_array Container>
		inline void pop_heap(Container& con)
		{
			namespace_heap::pop_heap(__begin_for_container(con), __end_for_container(con), _RANGES less {});
		}

		// sort_heap() for 仿函数 强化版
		template <__is_iterator_or_c_pointer RandomAccessIterator, typename Function>
		inline void sort_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun)
		{
			fun = __check_fun(fun);

			while (1 < last - first)
			{
				namespace_heap::pop_heap(first, last--, fun);
			}
		}

		// sort_heap() 标准版
		template <__is_iterator_or_c_pointer RandomAccessIterator>
		inline void sort_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			namespace_heap::sort_heap(first, last, _RANGES less {});
		}

		// sort_heap() for 容器、仿函数 强化版
		template <__is_container_or_c_array Container, typename Function>
		inline void sort_heap(Container& con, Function fun)
		{
			namespace_heap ::sort_heap(__begin_for_container(con), __end_for_container(con), fun);
		}

		// sort_heap() for 容器 强化版
		template <__is_container_or_c_array Container>
		inline void sort_heap(Container& con)
		{
			namespace_heap ::sort_heap(__begin_for_container(con), __end_for_container(con), _RANGES less {});
		}

		// make_heap() for 仿函数 强化版
		template <__is_iterator_or_c_pointer RandomAccessIterator, typename Function>
		inline void make_heap(RandomAccessIterator first, RandomAccessIterator last, Function fun)
		{
			fun = __check_fun(fun);

			using value_type	= __value_type_for_iter<RandomAccessIterator>;
			using distance_type = __difference_type_for_iter<RandomAccessIterator>;

			if (last - first < 2)
			{
				return;
			}

			distance_type len	 = last - first;
			distance_type parent = (len - 2) / 2;

			while (true)
			{
				namespace_heap::__adjust_heap(first, parent, len, __cove_type(*(first + parent), value_type), fun);

				if (parent == 0)
				{
					return;
				}

				parent--;
			}
		}

		// make_heap() 标准版
		template <__is_iterator_or_c_pointer RandomAccessIterator>
		inline void make_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			namespace_heap::make_heap(first, last, _RANGES less {});
		}

		// make_heap() for 仿函数、容器 强化版
		template <__is_container_or_c_array Container, typename Function>
		inline void make_heap(Container& con, Function fun)
		{
			namespace_heap::make_heap(__begin_for_container(con), __end_for_container(con), fun);
		}

		// make_heap() for 容器 强化版
		template <__is_container_or_c_array Container>
		inline void make_heap(Container& con)
		{
			namespace_heap::make_heap(__begin_for_container(con), __end_for_container(con), _RANGES less {});
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
