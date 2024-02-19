#pragma once

#include "./00_basicFile.h"

#ifdef _HAS_CXX20

_BEGIN_NAMESPACE_ZHANG

struct __Push_heap_aux_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	template <_STD random_access_iterator RandomAccessIterator,
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

		while ((topIndex < holeIndex) && _STD invoke(pred, _STD invoke(proj, *(first + parent)), value))
		{
			*(first + holeIndex) = *(first + parent);
			holeIndex			 = parent;
			parent				 = (holeIndex - 1) / 2;
		}

		*(first + holeIndex) = value;
	}
};

constexpr inline __Push_heap_aux_function __zh_Push_heap_aux { __Not_quite_object::__Construct_tag {} };

// 此处实现 push_heap() 算法
struct __Push_heap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	// push_heap() for 仿函数 标准版
	template <_STD random_access_iterator  RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		using value_type	  = typename _STD	   iter_value_t<RandomAccessIterator>;
		using difference_type = typename _STD iter_difference_t<RandomAccessIterator>;

		__zh_Push_heap_aux(first,
						   static_cast<difference_type>((last - first) - 1),
						   static_cast<difference_type>(0),
						   static_cast<value_type>(*(last - 1)),
						   _check_function(pred),
						   proj);
	}

	// push_heap() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Push_heap_function push_heap { __Not_quite_object::__Construct_tag {} };

// 此处实现 pop_heap() 算法
struct __Pop_heap_function: private __Not_quite_object
{
private:

	// pop_heap -- 辅助函数
	template <_STD random_access_iterator RandomAccessIterator,
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
			if (_STD invoke(pred,
							_STD invoke(proj, *(first + secondChild)),
							_STD invoke(proj, *(first + (secondChild - 1)))))
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
	template <_STD random_access_iterator RandomAccessIterator,
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
		using difference_type = typename _STD iter_difference_t<RandomAccessIterator>;

		*result = *first;

		__adjust_heap(first,
					  static_cast<difference_type>(0),
					  static_cast<difference_type>(last - first),
					  value,
					  pred,
					  proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// pop_heap() for 仿函数 标准版
	template <_STD random_access_iterator  RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		using value_type = typename _STD iter_value_t<RandomAccessIterator>;

		__default_pop_heap(first,
						   last - 1,
						   last - 1,
						   static_cast<value_type>(*(last - 1)),
						   _check_function(pred),
						   proj);
	}

	// pop_heap() for 容器、仿函数 强化版
	template <_RANGES range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Pop_heap_function pop_heap { __Not_quite_object::__Construct_tag {} };

// 此处实现 sort_heap() 算法
struct __Sort_heap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	// sort_heap() for 仿函数 标准版
	template <_STD random_access_iterator  RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		pred = _check_function(pred);

		while (1 < (last - first))
		{
			pop_heap(first, last--, pred, proj);
		}
	}

	// sort_heap() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Sort_heap_function sort_heap { __Not_quite_object::__Construct_tag {} };

// 此处实现 make_heap() 算法
struct __Make_heap_function: private __Not_quite_object
{
private:

	template <_STD random_access_iterator  RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	static constexpr void
		__default_make_heap(RandomAccessIterator first, RandomAccessIterator last, Predicate pred, Projection proj)
	{
		if ((last - first) < 2)
		{
			return;
		}

		pred = _check_function(pred);

		using value_type	= typename _STD	   iter_value_t<RandomAccessIterator>;
		using distance_type = typename _STD iter_difference_t<RandomAccessIterator>;

		distance_type len	 = last - first;
		distance_type parent = (len - 2) / 2;

		while (true)
		{
			__adjust_heap(first, parent, len, static_cast<value_type>(*(first + parent)), pred, proj);

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
	template <_STD random_access_iterator  RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const
	{
		__default_make_heap(first, last, _check_function(pred), proj);
	}

	// make_heap() for 仿函数、容器 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
	constexpr void operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Make_heap_function make_heap { __Not_quite_object::__Construct_tag {} };

_END_NAMESPACE_ZHANG

#endif	// _HAS_CXX20
