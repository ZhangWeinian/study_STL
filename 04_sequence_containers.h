#pragma once

#include "00_basicFile.h"

#ifdef __HAS_CPP20

__BEGIN_NAMESPACE_ZHANG


/* heap 算法 */
__BEGIN_NEW_NAMESPACE(np_heap)

// push_heap() -- 辅助函数
template <__is_random_access_iterator RandomAccessIterator,
		  typename Distance,
		  typename Type,
		  class Predicate	  = _RANGES	 less,
		  typename Projection = _STD identity>
inline void __push_heap(RandomAccessIterator first,
						Distance			 holeIndex,
						Distance			 topIndex,
						Type				 value,
						Predicate			 pred = {},
						Projection			 proj = {})
{
	Distance parent = (holeIndex - 1) / 2;

	while ((topIndex < holeIndex) && __invoke(pred, __invoke(proj, *(first + parent)), value))
	{
		*(first + holeIndex) = __invoke(proj, *(first + parent));
		holeIndex			 = parent;
		parent				 = (holeIndex - 1) / 2;
	}

	*(first + holeIndex) = value;
}

// push_heap() for 仿函数 标准版
template <__is_random_access_iterator RandomAccessIterator,
		  class Predicate	  = _RANGES	  less,
		  typename Projection = _STD  identity>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last, Predicate pred = {}, Projection proj = {})
{
	pred = __check_predicate(pred);

	using value_type = __value_type_for_iter<RandomAccessIterator>;
	using Distance	 = __difference_type_for_iter<RandomAccessIterator>;

	np_heap::__push_heap(first,
						 __cove_type((last - first) - 1, Distance),
						 __init_type(0, Distance),
						 __cove_type(*(last - 1), value_type),
						 pred,
						 proj);
}

// push_heap() for 容器、仿函数 强化版
template <__is_range Range, class Predicate = _RANGES less, typename Projection = _STD identity>
inline void push_heap(Range& con, Predicate pred = {}, Projection proj = {})
{
	np_heap::push_heap(__begin_for_range(con), __end_for_range(con), pred, proj);
}

// pop_heap -- 辅助函数
template <__is_random_access_iterator RandomAccessIterator,
		  typename Distance,
		  typename Type,
		  class Predicate	  = _RANGES	 less,
		  typename Projection = _STD identity>
inline void __adjust_heap(RandomAccessIterator first,
						  Distance			   holeIndex,
						  Distance			   len,
						  Type				   value,
						  Predicate			   pred = {},
						  Projection		   proj = {})
{
	Distance topIndex { holeIndex };
	Distance secondChild { 2 * holeIndex + 2 };

	while (secondChild < len)
	{
		if (__invoke(pred, __invoke(proj, *(first + secondChild)), __invoke(proj, *(first + (secondChild - 1)))))
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

	np_heap::__push_heap(first, holeIndex, topIndex, value, pred, proj);
}

// pop_heap() -- 辅助函数
template <__is_random_access_iterator RandomAccessIterator,
		  typename Type,
		  class Predicate	  = _RANGES	 less,
		  typename Projection = _STD identity>
inline void __pop_heap(RandomAccessIterator first,
					   RandomAccessIterator last,
					   RandomAccessIterator result,
					   Type					value,
					   Predicate			pred = {},
					   Projection			proj = {})
{
	using distance_type = __difference_type_for_iter<RandomAccessIterator>;

	*result = __invoke(proj, *first);

	np_heap::__adjust_heap(first,
						   __init_type(0, distance_type),
						   __cove_type(last - first, distance_type),
						   value,
						   pred,
						   proj);
}

// pop_heap() for 仿函数 标准版
template <__is_random_access_iterator RandomAccessIterator,
		  class Predicate	  = _RANGES	  less,
		  typename Projection = _STD  identity>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last, Predicate pred = {}, Projection proj = {})
{
	pred = __check_predicate(pred);

	using value_type = __value_type_for_iter<RandomAccessIterator>;

	np_heap::__pop_heap(first, last - 1, last - 1, __cove_type(*(last - 1), value_type), pred, proj);
}

// pop_heap() for 容器、仿函数 强化版
template <__is_range Range, class Predicate = _RANGES less, typename Projection = _STD identity>
inline void pop_heap(Range& con, Predicate pred = {}, Projection proj = {})
{
	np_heap::pop_heap(__begin_for_range(con), __end_for_range(con), pred, proj);
}

// sort_heap() for 仿函数 标准版
template <__is_random_access_iterator RandomAccessIterator,
		  class Predicate	  = _RANGES	  less,
		  typename Projection = _STD  identity>
inline void sort_heap(RandomAccessIterator first, RandomAccessIterator last, Predicate pred = {}, Projection proj = {})
{
	pred = __check_predicate(pred);

	while (1 < (last - first))
	{
		np_heap::pop_heap(first, last--, pred, proj);
	}
}

// sort_heap() for 容器、仿函数 强化版
template <__is_range Range, class Predicate = _RANGES less, typename Projection = _STD identity>
inline void sort_heap(Range& con, Predicate pred = {}, Projection proj = {})
{
	np_heap ::sort_heap(__begin_for_range(con), __end_for_range(con), pred, proj);
}

// make_heap() for 仿函数 标准版
template <__is_random_access_iterator RandomAccessIterator,
		  class Predicate	  = _RANGES	  less,
		  typename Projection = _STD  identity>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last, Predicate pred = {}, Projection proj = {})
{
	if ((last - first) < 2)
	{
		return;
	}

	pred = __check_predicate(pred);

	using value_type	= __value_type_for_iter<RandomAccessIterator>;
	using distance_type = __difference_type_for_iter<RandomAccessIterator>;

	distance_type len	 = last - first;
	distance_type parent = (len - 2) / 2;

	while (true)
	{
		np_heap::__adjust_heap(first, parent, len, __cove_type(*(first + parent), value_type), pred, proj);

		if (parent == 0)
		{
			return;
		}

		parent--;
	}
}

// make_heap() for 仿函数、容器 强化版
template <__is_range Range, class Predicate = _RANGES less, typename Projection = _STD identity>
inline void make_heap(Range& con, Predicate pred = {}, Projection proj = {})
{
	np_heap::make_heap(__begin_for_range(con), __end_for_range(con), pred, proj);
}


__END_NEW_NAMESPACE(np_heap)

// 对外接口
using np_heap::make_heap;
using np_heap::pop_heap;
using np_heap::push_heap;
using np_heap::sort_heap;


__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
