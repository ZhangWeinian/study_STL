#pragma once

#include "00_basicFile.h"

namespace zhang::sequence_containers
{
	// 预定义一些用于 简写 和 标志识别 的宏
#ifndef __zh_namespace

	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#define __zh_namespace ::zhang::

#endif // !__zh_namespace


	/* heap 算法 */
	namespace namespace_heap
	{
		// push_heap
		template <typename RandomAccessIterator, typename Distance, typename T>
		inline void __push_heap(RandomAccessIterator first, Distance holeIndex, Distance topIndex, T value)
		{
			Distance parent = (holeIndex - 1) / 2;

			while (holeIndex > topIndex && *(first + parent) < value)
			{
				*(first + holeIndex) = *(first + parent);
				holeIndex			 = parent;
				parent				 = (holeIndex - 1) / 2;
			}

			*(first + holeIndex) = value;
		}

		template <typename RandomAccessIterator>
		inline void push_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			using value_type = __value_type_for_iter<RandomAccessIterator>;
			using Distance	 = __difference_type_for_iter<RandomAccessIterator>;

			namespace_heap::__push_heap(first,
										__cove_type((last - first) - 1, Distance),
										__init_type(0, Distance),
										__cove_type(*(last - 1), value_type));
		}

		// pop_heap
		template <typename RandomAccessIterator, typename Distance, typename T>
		inline void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value)
		{
			Distance topIndex	 = holeIndex;
			Distance secondChild = 2 * holeIndex + 2;

			while (secondChild < len)
			{
				if (*(first + (secondChild - 1)) > *(first + secondChild))
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

			namespace_heap::__push_heap(first, holeIndex, topIndex, value);
		}

		template <typename RandomAccessIterator, typename T>
		inline void
			__pop_heap(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator result, T value)
		{
			using distance_type = __difference_type_for_iter<RandomAccessIterator>;

			*result = *first;

			namespace_heap::__adjust_heap(first,
										  __init_type(0, distance_type),
										  __cove_type(last - first, distance_type),
										  value);
		}

		template <typename RandomAccessIterator>
		inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			using value_type = __value_type_for_iter<RandomAccessIterator>;

			namespace_heap::__pop_heap(first, last - 1, last - 1, __cove_type(*(last - 1), value_type));
		}

		// sort_heap
		template <typename RandomAccessIterator>
		inline void sort_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			while (last - first > 1)
			{
				::zhang::sequence_containers::namespace_heap::pop_heap(first, last--);
			}
		}

		// make_heap
		template <typename RandomAccessIterator>
		inline void __make_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
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
				namespace_heap::__adjust_heap(first, parent, len, __cove_type(*(first + parent), value_type));

				if (parent == 0)
				{
					return;
				}

				parent--;
			}
		}

		template <typename RandomAccessIterator>
		inline void make_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			namespace_heap::__make_heap(first, last);
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
} // namespace zhang::sequence_containers
