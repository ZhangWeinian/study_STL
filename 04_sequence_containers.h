#pragma once

#include "00_basicFile.h"

namespace zhang::sequence_containers
{
	// 预定义一些用于 简写 和 标志识别 的宏
#ifndef __ZH_NAMESPACE__

	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#define __ZH_NAMESPACE__ ::zhang::
	#define __ZH_ITER__		 ::zhang::iterator::namespace_iterator::

#endif // !__ZH_NAMESPACE__


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

		template <typename RandomAccessIterator, typename Distance, typename T>
		inline void __push_heap_aux(RandomAccessIterator first, RandomAccessIterator last, Distance*, T*)
		{
			namespace_heap::__push_heap(first,
										_cove_type((last - first) - 1, Distance),
										_init_type(0, Distance),
										_cove_type(*(last - 1), T));
		}

		template <typename RandomAccessIterator>
		inline void push_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			namespace_heap::__push_heap_aux(first,
											last,
											__ZH_ITER__ distance_type(first),
											__ZH_ITER__ value_type(first));
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

		template <typename RandomAccessIterator, typename T, typename Distance>
		inline void __pop_heap(RandomAccessIterator first,
							   RandomAccessIterator last,
							   RandomAccessIterator result,
							   T					value,
							   Distance*)
		{
			*result = *first;

			namespace_heap::__adjust_heap(first, _init_type(0, Distance), _cove_type(last - first, Distance), value);
		}

		template <typename RandomAccessIterator, typename T>
		inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*)
		{
			namespace_heap::__pop_heap(first,
									   last - 1,
									   last - 1,
									   _cove_type(*(last - 1), T),
									   __ZH_ITER__ distance_type(first));
		}

		template <typename RandomAccessIterator>
		inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			namespace_heap::__pop_heap_aux(first, last, __ZH_ITER__ value_type(first));
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
		template <typename RandomAccessIterator, typename T, typename Distance>
		inline void __make_heap(RandomAccessIterator first, RandomAccessIterator last, T*, Distance*)
		{
			if (last - first < 2)
			{
				return;
			}

			Distance len	= last - first;
			Distance parent = (len - 2) / 2;

			while (true)
			{
				namespace_heap::__adjust_heap(first, parent, len, _cove_type(*(first + parent), T));

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
			namespace_heap::__make_heap(first, last, __ZH_ITER__ value_type(first), __ZH_ITER__ distance_type(first));
		}
	} // namespace namespace_heap

	// 对外接口
	using namespace_heap::make_heap;
	using namespace_heap::pop_heap;
	using namespace_heap::push_heap;
	using namespace_heap::sort_heap;


#ifdef __ZH_NAMESPACE__
	#undef __ZH_NAMESPACE__
	#undef __ZH_ITER__
#endif // __ZH_NAMESPACE__
} // namespace zhang::sequence_containers
