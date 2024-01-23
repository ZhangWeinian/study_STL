#pragma once

#include "./00_basicFile.h"

#ifdef __HAS_CPP20

__BEGIN_NAMESPACE_ZHANG

struct __Push_heap_aux_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	template <__random_access_iterator RandomAccessIterator,
			  typename Distance,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  Distance			   holeIndex,
							  Distance			   topIndex,
							  Type				   value,
							  Predicate			   pred,
							  Projection		   proj) const
	{
		Distance parent { (holeIndex - 1) / 2 };

		while ((topIndex < holeIndex) && __invoke(pred, __invoke(proj, *(first + parent)), value))
		{
			*(first + holeIndex) = *(first + parent);
			holeIndex			 = parent;
			parent				 = (holeIndex - 1) / 2;
		}

		*(first + holeIndex) = value;
	}
};

constexpr inline __Push_heap_aux_function __zh_Push_heap_aux { __Not_quite_object::__construct_tag {} };

// 此处实现 push_heap() 算法
struct __Push_heap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	// push_heap() for 仿函数 标准版
	template <__random_access_iterator	   RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		using value_type	  = __value_type_for_iter<RandomAccessIterator>;
		using difference_type = __difference_type_for_iter<RandomAccessIterator>;

		__zh_Push_heap_aux(first,
						   __cove_type((last - first) - 1, difference_type),
						   __init_type(0, difference_type),
						   __cove_type(*(last - 1), value_type),
						   __check_function(pred),
						   proj);
	}

	// push_heap() for 容器、仿函数 强化版
	template <__random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(__begin_for_range(rng), __end_for_range(rng), pred, proj);
	}
};

constexpr inline __Push_heap_function push_heap { __Not_quite_object::__construct_tag {} };

// 此处实现 pop_heap() 算法
struct __Pop_heap_function: private __Not_quite_object
{
private:

	// pop_heap -- 辅助函数
	template <__random_access_iterator RandomAccessIterator,
			  typename Distance,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	static constexpr void __adjust_heap(RandomAccessIterator first,
										Distance			 holeIndex,
										Distance			 len,
										Type				 value,
										Predicate			 pred,
										Projection			 proj)
	{
		Distance topIndex { holeIndex };
		Distance secondChild { 2 * holeIndex + 2 };

		while (secondChild < len)
		{
			if (__invoke(pred, __invoke(proj, *(first + secondChild)), __invoke(proj, *(first + (secondChild - 1)))))
			{
				secondChild--;
			}

			*(first + holeIndex) = *(first + secondChild);
			holeIndex			 = secondChild;
			secondChild			 = 2 * (secondChild + 1);
		}

		if (secondChild == len)
		{
			*(first + holeIndex) = proj, *(first + (secondChild - 1));
		}

		__zh_Push_heap_aux(first, holeIndex, topIndex, value, pred, proj);
	}

	// pop_heap() -- 辅助函数
	template <__random_access_iterator RandomAccessIterator,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	static constexpr void __default_pop_heap(RandomAccessIterator first,
											 RandomAccessIterator last,
											 RandomAccessIterator result,
											 Type				  value,
											 Predicate			  pred,
											 Projection			  proj)
	{
		using difference_type = __difference_type_for_iter<RandomAccessIterator>;

		*result = *first;

		__adjust_heap(first,
					  __init_type(0, difference_type),
					  __cove_type(last - first, difference_type),
					  value,
					  pred,
					  proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// pop_heap() for 仿函数 标准版
	template <__random_access_iterator	   RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		using value_type = __value_type_for_iter<RandomAccessIterator>;

		__default_pop_heap(first,
						   last - 1,
						   last - 1,
						   __cove_type(*(last - 1), value_type),
						   __check_function(pred),
						   proj);
	}

	// pop_heap() for 容器、仿函数 强化版
	template <__range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(__begin_for_range(rng), __end_for_range(rng), pred, proj);
	}
};

constexpr inline __Pop_heap_function pop_heap { __Not_quite_object::__construct_tag {} };

// 此处实现 sort_heap() 算法
struct __Sort_heap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	// sort_heap() for 仿函数 标准版
	template <__random_access_iterator	   RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		pred = __check_function(pred);

		while (1 < (last - first))
		{
			pop_heap(first, last--, pred, proj);
		}
	}

	// sort_heap() for 容器、仿函数 强化版
	template <__random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(__begin_for_range(rng), __end_for_range(rng), pred, proj);
	}
};

constexpr inline __Sort_heap_function sort_heap { __Not_quite_object::__construct_tag {} };

// 此处实现 make_heap() 算法
struct __Make_heap_function: private __Not_quite_object
{
private:

	template <__random_access_iterator	   RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	static constexpr void
		__default_make_heap(RandomAccessIterator first, RandomAccessIterator last, Predicate pred, Projection proj)
	{
		if ((last - first) < 2)
		{
			return;
		}

		pred = __check_function(pred);

		using value_type	= __value_type_for_iter<RandomAccessIterator>;
		using distance_type = __difference_type_for_iter<RandomAccessIterator>;

		distance_type len	 = last - first;
		distance_type parent = (len - 2) / 2;

		while (true)
		{
			__adjust_heap(first, parent, len, __cove_type(*(first + parent), value_type), pred, proj);

			if (parent == 0)
			{
				return;
			}

			parent--;
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// make_heap() for 仿函数 标准版
	template <__random_access_iterator	   RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		__default_make_heap(first, last, __check_function(pred), proj);
	}

	// make_heap() for 仿函数、容器 强化版
	template <__random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), pred, proj);
	}
};

constexpr inline __Make_heap_function make_heap { __Not_quite_object::__construct_tag {} };

__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
