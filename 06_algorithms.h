#pragma once

#include "00_basicFile.h"


#ifdef __HAS_CPP20

__BEGIN_NP_ZHANG

// 此仿函数直接服务于 copy()，它分为一个 完全泛化版本 和两个 偏特化版本
struct __copy_dispatch_function: private __not_quite_object
{
public:

	/* function __copy_with_random_access_iter() -- 辅助函数 */
	template <__is_random_access_iterator RandomAccessIterator, __is_output_iterator OutputIterator>
	static constexpr OutputIterator __copy_with_random_access_iter(RandomAccessIterator first,
																   RandomAccessIterator last,
																   OutputIterator		result) noexcept
	{
		using difference_type = __difference_type_for_iter<RandomAccessIterator>;

		for (difference_type i { last - first }; i > 0; --i, ++result, ++first) // 以 i 决定循环的次数 -- 速度快
		{
			*result* first;
		}

		return result;
	}

public:

	using __not_quite_object::__not_quite_object;

	// 完全泛化版本
	template <__is_input_iterator InputIterator, __is_output_iterator OutputIterator>
	constexpr OutputIterator operator()(InputIterator first, InputIterator last, OutputIterator result) noexcept
	{
		using iter_type_tag = __type_tag_for_iter<InputIterator>;
		constexpr bool is_random_access_iter { _STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()> };

		// 此处兵分两路：根据迭代器种类的不同，调用不同的 __copy_with_iter_tag() ，为的是不同种类的迭代器所使用的循环条件不同，有快慢之分
		if constexpr (is_random_access_iter)
		{
			// 此处单独划分出一个函数，为的是其他地方也能用到
			return __copy_with_random_access_iter(__move(first), __move(last), __move(result));
		}
		else
		{
			for (; first != last; ++result, ++first) // 以迭代器相同与否，决定循环是否继续 -- 速度慢
			{
				*result = *first;
			}

			return result;
		}
	}

	// 偏特化版本1：第一个参数是 const T* 指针形式，第二个参数是 T* 指针形式。另一个偏特化版本两个参数都是 T* 类型的，愚以为没必要…… 具体实现见书 319 页。
	template <typename T>
	constexpr T* operator()(const T* first, const T* last, T* result) noexcept
	{
		using pointer_type_tag = typename np_iterator::__type_traits<T>::has_trivial_assignment_operator;
		constexpr bool is_trivial_assignment_operator { _STD is_same_v<_STD true_type(), pointer_type_tag()> };

		if (is_trivial_assignment_operator) // 以下版本适用于 “指针所指之对象，具备 trivial assignment operator ”
		{
			__invoke(memmove, result, first, sizeof(T) * (last - first));

			return result + (last - first);
		}
		else // 以下版本适用于 “指针所指之对象，具备 non-trivial assignment operator ”
		{
			return __copy_with_random_access_iter(__move(first), __move(last), __move(result));
		}
	}
};

constexpr inline __copy_dispatch_function __copy_dispatch { __not_quite_object::__construct_tag {} };

/* 此处实现 copy() */
struct __copy_function: private __not_quite_object
{
private:

	// 统一调用方式
	/* function copy() 重载1 */
	static constexpr char*
		__default_cpoy(const char* first,
					   const char* last,
					   char* result) noexcept // 针对原生指针(可视为一种特殊的迭代器) const char* ，机型内存直接拷贝操作
	{
		__invoke(memmove, result, first, last - first);

		return result + (last - first);
	}

	/* function copy() 重载2 */
	static constexpr wchar_t* __default_cpoy(
		const wchar_t* first,
		const wchar_t* last,
		wchar_t* result) noexcept // 针对原生指针（可视为一种特殊的迭代器）const wchar_t* ，执行内存直接拷贝操作
	{
		__invoke(memmove, result, first, sizeof(wchar_t) * (last - first));

		return result + (last - first);
	}

	/* function copy() 一般泛型 */
	template <__is_input_iterator  InputIterator,
			  __is_output_iterator OutputIterator> // 完全泛化版本
	static constexpr OutputIterator
		__default_cpoy(InputIterator first, InputIterator last, OutputIterator result) noexcept
	{
		return __copy_dispatch(__move(first), __move(last), __move(result));
	}


public:

	using __not_quite_object::__not_quite_object;

	/* function copy() 一般泛型 */
	template <__is_input_iterator  InputIterator,
			  __is_output_iterator OutputIterator> // 完全泛化版本
	constexpr OutputIterator operator()(InputIterator first, InputIterator last, OutputIterator result) noexcept
	{
		return __default_cpoy(__move(first), __move(last), __move(result));
	}

	/* function copy() for all_容器 强化版 */
	template <__is_input_range Range1, __is_output_range Range2>
	constexpr auto operator()(const Range1& rng1, Range2& rng2) noexcept
	{
		return __default_cpoy(__begin_for_range_with_move(rng1),
							  __end_for_range_with_move(rng1),
							  __begin_for_range_with_move(rng2));
	}

	/* function copy() for from_容器 强化版 */
	template <__is_input_range Range, __is_output_iterator OutputIterator>
	constexpr auto operator()(const Range&& rng, OutputIterator result) noexcept
	{
		return __default_cpoy(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), __move(result));
	}
};

constexpr inline __copy_function copy { __not_quite_object::__construct_tag {} };

/* 此处实现一些简单函数 */

// 此处实现 accumulate()
struct __accumulate_function
{
private:

	// 统一调用方式
	template <__is_input_iterator InputIterator, typename T, typename Function, typename Projection = _STD identity>
	_NODISCARD static constexpr T __default_accumulate(InputIterator first,
													   InputIterator last,
													   T			 init,
													   Function fun				= _STD plus {},
													   Projection		   proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		for (; first != last; ++first)
		{
			init = __invoke(fun, init, __invoke(proj, *first));
		}

		return init;
	}

public:

	/* function accumulate() for 仿函数 标准版 */
	template <__is_input_iterator InputIterator, typename T, typename Function, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr T operator()(InputIterator first,
						   InputIterator last,
						   T			 init,
						   Function fun				= _STD plus {},
						   Projection		   proj = {}) noexcept
	{
		return __default_accumulate(__move(first), __move(last), __move(init), fun, proj);
	}

	/* function accumulate() for 容器、仿函数 强化版 */
	template <__is_input_range Range, typename T, typename Function, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr T operator()(const Range&& rng, T init, Function fun = _STD plus {}, Projection proj = {}) noexcept
	{
		return __default_accumulate(__begin_for_range_with_move(rng),
									__end_for_range_with_move(rng),
									__move(init),
									fun,
									proj);
	}
};

constexpr inline __accumulate_function accumulate {};

// 此处实现 count()
struct __count_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_input_iterator InputIterator,
			  typename T,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr auto __default_count(InputIterator first,
													 InputIterator last,
													 const T&	   value,
													 Function	   fun	= {},
													 Projection	   proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		using difference_type = __difference_type_for_iter<InputIterator>;

		difference_type n = 0;
		for (; first != last; ++first)
		{
			if (__invoke(fun, __invoke(proj, *first), value))
			{
				++n;
			}
		}

		return n;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function count() for 仿函数 标准版 */
	template <__is_input_iterator InputIterator,
			  typename T,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(InputIterator first,
							  InputIterator last,
							  const T&		value,
							  Function		fun	 = {},
							  Projection	proj = {}) noexcept
	{
		return __default_count(__move(first), __move(last), __move(value), fun, proj);
	}

	/* function count() for 容器、仿函数 强化版 */
	template <__is_input_range Range,
			  typename T,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, const T& value, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_count(__begin_for_range_with_move(rng),
							   __end_for_range_with_move(rng),
							   __move(value),
							   fun,
							   proj);
	}
};

constexpr inline __count_function count { __not_quite_object::__construct_tag {} };

// 此处实现 itoa()
struct __itoa_function
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator, typename T>
	static constexpr void __default_itoa(ForwardIterator first, ForwardIterator last, T value) noexcept
	{
		while (first != last)
		{
			*first++ = value++;
		}
	}

public:

	/* function itoa() 标准版 */
	template <__is_forward_iterator ForwardIterator, typename T>
	constexpr void operator()(ForwardIterator first, ForwardIterator last, T value) noexcept
	{
		__default_itoa(__move(first), __move(last), __move(value));
	}

	/* function itoa() for 容器 加强版 */
	template <__is_forward_range Range, typename T>
	constexpr void operator()(Range&& rng, T value) noexcept
	{
		__default_itoa(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), __move(value));
	}
};

constexpr inline __itoa_function itoa {};

// 此处实现 find()
struct __find_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_input_iterator InputIterator,
			  typename T,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr InputIterator __default_find(InputIterator first,
															 InputIterator last,
															 const T&	   value,
															 Function	   fun	= {},
															 Projection	   proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		while ((first != last) && __invoke(fun, __invoke(proj, *first), value))
		{
			++first;
		}

		return first;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function find() for 仿函数 标准版 */
	template <__is_input_iterator InputIterator,
			  typename T,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr InputIterator operator()(InputIterator first,
									   InputIterator last,
									   const T&		 value,
									   Function		 fun  = {},
									   Projection	 proj = {}) noexcept
	{
		return __default_find(__move(first), __move(last), __move(value), fun, proj);
	}

	/* function find() for 仿函数、容器 强化版 */
	template <__is_input_range Range,
			  typename T,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(const Range&& rng, const T& value, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_find(__begin_for_range_with_move(rng),
							  __end_for_range_with_move(rng),
							  __move(value),
							  fun,
							  proj);
	}
};

constexpr inline __find_function find { __not_quite_object::__construct_tag {} };

// 此处实现 find_first_of()
struct __find_first_of_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_input_iterator		 InputIterator,
			  __is_forward_iterator		 ForwardIterator,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr InputIterator __default_find_first_of(InputIterator	  first1,
																	  InputIterator	  last1,
																	  ForwardIterator first2,
																	  ForwardIterator last2,
																	  Function		  fun  = {},
																	  Projection	  proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		for (; first1 != last1; ++first1)
		{
			for (ForwardIterator i = first2; i != last2; ++i)
			{
				if (__invoke(fun, __invoke(proj, *first1), *i)) // 如果序列 1 的元素与序列 2 中元素相等
				{
					return first1;
				}
			}
		}

		return last1;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function find_first_of() for 仿函数 标准版 */
	template <__is_input_iterator		 InputIterator,
			  __is_forward_iterator		 ForwardIterator,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr InputIterator operator()(InputIterator   first1,
									   InputIterator   last1,
									   ForwardIterator first2,
									   ForwardIterator last2,
									   Function		   fun	= {},
									   Projection	   proj = {}) noexcept
	{
		return __default_find_first_of(__move(first1), __move(last1), __move(first2), __move(last2), fun, proj);
	}

	/* function find_first_of() for 容器、仿函数 强化版 */
	template <__is_input_range			 Range1,
			  __is_forward_range		 Range2,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(const Range1& rng1, const Range2& rng2, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_find_first_of(__begin_for_range_with_move(rng1),
									   __end_for_range_with_move(rng1),
									   __begin_for_range_with_move(rng2),
									   __end_for_range_with_move(rng2),
									   fun,
									   proj);
	}
};

constexpr inline __find_first_of_function find_first_of { __not_quite_object::__construct_tag {} };

// 此处实现 swap()
struct __swap_function
{
public:

	template <typename T>
	constexpr void operator()(T& a, T& b) const noexcept
	{
		T tmp = a;
		a	  = b;
		b	  = tmp;
	}
};

constexpr inline __swap_function swap {};

// 此处实现 iter_swap()
struct __iter_swap_function
{
public:

	template <__is_forward_iterator ForwardIterator1, __is_forward_iterator ForwardIterator2>
	constexpr void operator()(ForwardIterator1 a, ForwardIterator2 b) noexcept
	{
		using value_type = __value_type_for_iter<ForwardIterator1>;

		value_type tmp = *a;
		*a			   = *b;
		*b			   = tmp;
	}
};

constexpr inline __iter_swap_function iter_swap {};

// 此处实现 swap_ranges()
struct __swap_ranges_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator1, __is_forward_iterator ForwardIterator2>
	static constexpr ForwardIterator2
		__default_swap_ranges(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2) noexcept
	{
		for (; first1 != last1; ++first1, ++first2)
		{
			iter_swap(first1, first2);
		}

		return first2;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function swap_ranges() 标准版 */
	template <__is_forward_iterator ForwardIterator1, __is_forward_iterator ForwardIterator2>
	constexpr ForwardIterator2
		operator()(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2) noexcept
	{
		return __default_swap_ranges(__move(first1), __move(last1), __move(first2));
	}

	/* function swap_ranges() for all_容器 强化版 */
	template <__is_forward_range Range>
	constexpr auto operator()(Range&& rng1, Range&& rng2) noexcept
	{
		return __default_swap_ranges(__begin_for_range_with_move(rng1),
									 __end_for_range_with_move(rng1),
									 __begin_for_range_with_move(rng2));
	}

	/* function swap_ranges() for from_容器 强化版 */
	template <__is_forward_range Range, __is_forward_iterator Iterator>
	constexpr auto operator()(Range&& rng, Iterator result) noexcept
	{
		return __default_swap_ranges(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), __move(result));
	}
};

constexpr inline __swap_ranges_function swap_ranges { __not_quite_object::__construct_tag {} };

// 此处实现 for_each()
struct __for_each_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_input_iterator InputIterator, typename Function>
	static constexpr Function __default_for_each(InputIterator first, InputIterator last, Function fun) noexcept
	{
		fun = __global_check_fun(fun);

		for (; first != last; ++first)
		{
			__invoke(fun, *first);
		}

		return fun;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function for_each() 标准版 */
	template <__is_input_iterator InputIterator, typename Function>
	constexpr Function operator()(InputIterator first, InputIterator last, Function fun) noexcept
	{
		return __default_for_each(__move(first), __move(last), __move(fun));
	}

	/* function for_each() for 容器 强化版 */
	template <__is_input_range Range, typename Function>
	inline Function for_each(Range&& rng, Function fun) noexcept
	{
		return __default_for_each(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun);
	}
};

constexpr inline __for_each_function for_each { __not_quite_object::__construct_tag {} };

// 此处实现 equal()
struct __equal_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_input_iterator		 InputIterator1,
			  __is_input_iterator		 InputIterator2,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr bool __default_equal(InputIterator1 first1,
													 InputIterator1 last1,
													 InputIterator2 first2,
													 Function		fun	  = {},
													 Projection		proj1 = {},
													 Projection		proj2 = {}) noexcept
	{
		fun = __global_check_fun(fun);

		for (; first1 != last1; ++first1, ++first2) // 如果序列 1 的元素数量多于序列 2 的元素数量, 那就糟糕了
		{
			if (!(__invoke(fun, __invoke(proj1, *first1), __invoke(proj2, *first2))))
			{
				return false;
			}
		}

		return true;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function equal() for 仿函数 标准版 */
	template <__is_input_iterator		 InputIterator1,
			  __is_input_iterator		 InputIterator2,
			  class Function	  = _RANGES	  equal_to,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr bool operator()(InputIterator1 first1,
							  InputIterator1 last1,
							  InputIterator2 first2,
							  Function		 fun   = {},
							  Projection	 proj1 = {},
							  Projection	 proj2 = {}) noexcept
	{
		return __default_equal(__move(first1), __move(last1), __move(first2), fun, proj1, proj2);
	}

	/* function equal() for 容器、仿函数 强化版 */
	template <__is_input_range Range, class Function = _RANGES equal_to, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr bool
		operator()(Range&& rng1, Range&& rng2, Function fun = {}, Projection proj1 = {}, Projection proj2 = {}) noexcept
	{
		return __default_equal(__begin_for_range_with_move(rng1),
							   __end_for_range_with_move(rng1),
							   __begin_for_range_with_move(rng2),
							   fun,
							   proj1,
							   proj2);
	}
};

constexpr inline __equal_function equal { __not_quite_object::__construct_tag {} };

// 此处实现 fill()
struct __fill_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator, typename T>
	static constexpr void __default_fill(ForwardIterator first, ForwardIterator last, const T& value) noexcept
	{
		for (; first != last; ++first)
		{
			*first = value;
		}
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function fill() 标准版 */
	template <__is_forward_iterator ForwardIterator, typename T>
	constexpr void operator()(ForwardIterator first, ForwardIterator last, const T& value) noexcept
	{
		__default_fill(__move(first), __move(last), __move(value));
	};
};

constexpr inline __fill_function fill { __not_quite_object::__construct_tag {} };

// 此处实现 fill_n()
struct __fill_n_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_output_iterator OutputIterator, typename Size, typename T>
	static constexpr OutputIterator __default_fill_n(OutputIterator first, Size n, const T& value) noexcept
	{
		for (; n > 0; --n, ++first)
		{
			*first = value;
		}

		return first;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function fill_n() 标准版 */
	template <__is_output_iterator OutputIterator, typename Size, typename T>
	constexpr OutputIterator operator()(OutputIterator first, Size n, const T& value) noexcept
	{
		return __default_fill_n(__move(first), __move(n), __move(value));
	};
};

constexpr inline __fill_n_function fill_n { __not_quite_object::__construct_tag {} };

// 此处实现 max()
struct __max_function: private __not_quite_object
{
public:

	using __not_quite_object::__not_quite_object;

	/* function max() for 仿函数 标准版 */
	template <typename T, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr const T& operator()(const T& a, const T& b, Function fun = {}, Projection proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		return __invoke(fun, __invoke(proj, a), __invoke(proj, b)) ? b : a;
	}
};

constexpr inline __max_function max { __not_quite_object::__construct_tag {} };

// 此处实现 min()
struct __min_function: private __not_quite_object
{
public:

	using __not_quite_object::__not_quite_object;

	/* function min() for 仿函数 标准版 */
	template <typename T, class Function = _RANGES greater, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr const T& operator()(const T& a, const T& b, Function fun = {}, Projection proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		return __invoke(fun, __invoke(proj, a), __invoke(proj, b)) ? b : a;
	}
};

constexpr inline __min_function min { __not_quite_object::__construct_tag {} };

// 此处实现 max_element()
struct __max_element_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD static constexpr ForwardIterator __default_max_element(ForwardIterator first,
																	  ForwardIterator last,
																	  Function		  fun  = {},
																	  Projection	  proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		if (first == last)
		{
			return first;
		}

		ForwardIterator result { first };
		while (++first != last)
		{
			if (__invoke(fun, __invoke(proj, *result), __invoke(proj, *first)))
			{
				result = first;
			}
		}

		return result;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function max_element() for 仿函数 标准版 */
	template <__is_forward_iterator ForwardIterator, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr ForwardIterator
		operator()(ForwardIterator first, ForwardIterator last, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_max_element(__move(first), __move(last), fun, proj);
	}

	/* function max_element() for 容器、仿函数 强化版 */
	template <__is_forward_range Range, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_max_element(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __max_element_function max_element { __not_quite_object::__construct_tag {} };

// 此处实现 min_element()
struct __min_element_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator		 ForwardIterator,
			  class Function	  = _RANGES	  greater,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr ForwardIterator __default_min_element(ForwardIterator first,
																	  ForwardIterator last,
																	  Function		  fun  = {},
																	  Projection	  proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		if (first == last)
		{
			return first;
		}

		ForwardIterator result { first };
		while (++first != last)
		{
			if (__invoke(fun, __invoke(proj, *result), __invoke(proj, *first)))
			{
				result = first;
			}
		}

		return result;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function min_element() for 仿函数 标准版 */
	template <__is_forward_iterator		 ForwardIterator,
			  class Function	  = _RANGES	  greater,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr ForwardIterator
		operator()(ForwardIterator first, ForwardIterator last, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_min_element(__move(first), __move(last), fun, proj);
	}

	/* function min_element() for 容器、仿函数 强化版 */
	template <__is_forward_range Range, class Function = _RANGES greater, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_min_element(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __min_element_function min_element { __not_quite_object::__construct_tag {} };

// 此处实现 merge()
struct __merge_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_input_iterator		 InputIterator1,
			  __is_input_iterator		 InputIterator2,
			  __is_output_iterator		 OutputIterator,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr OutputIterator __default_merge(InputIterator1 first1,
															   InputIterator1 last1,
															   InputIterator2 first2,
															   InputIterator2 last2,
															   OutputIterator result,
															   Function		  fun  = {},
															   Projection	  proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		while ((first1 != last1) && (first2 != last2)) // 若两个序列都未完成，则继续
		{
			if (__invoke(fun, __invoke(proj, *first2), __invoke(proj, *first1))) // 若序列 2 的元素比较小
			{
				*result = *first2;												 // 则记录序列 2 的元素
				++first2;														 // 同时序列 2 前进 1 位
			}
			else																 // 反之同理
			{
				*result = *first1;
				++first1;
			}

			++result; // 每记录一个元素，result 前进 1 位
		}

		// 最后剩余元素拷贝到目的端。（以下两个序列一定至少有一个为空）
		return copy(__move(first2), __move(last2), copy(__move(first1), __move(last1), __move(result)));
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function merge() for 仿函数 标准版 */
	template <__is_input_iterator		 InputIterator1,
			  __is_input_iterator		 InputIterator2,
			  __is_output_iterator		 OutputIterator,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr OutputIterator operator()(InputIterator1 first1,
										InputIterator1 last1,
										InputIterator2 first2,
										InputIterator2 last2,
										OutputIterator result,
										Function	   fun	= {},
										Projection	   proj = {}) noexcept
	{
		return __default_merge(__move(first1), __move(last1), __move(first2), __move(last2), __move(result), fun, proj);
	}

	/* function merge() for 容器、仿函数 强化版 */
	template <__is_input_range			 Range1,
			  __is_input_range			 Range2,
			  __is_output_iterator		 OutputIterator,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(const Range1&	 rng1,
							  const Range2&	 rng2,
							  OutputIterator result,
							  Function		 fun  = {},
							  Projection	 proj = {}) noexcept
	{
		return __default_merge(__begin_for_range_with_move(rng1),
							   __end_for_range_with_move(rng1),
							   __begin_for_range_with_move(rng2),
							   __end_for_range_with_move(rng2),
							   __move(result),
							   fun,
							   proj);
	}
};

constexpr inline __merge_function merge { __not_quite_object::__construct_tag {} };

// 此处实现 transform()
struct __transform_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_input_iterator InputIterator1, __is_output_iterator OutputIterator, class Function = _STD identity>
	_NODISCARD static constexpr OutputIterator __default_transform(InputIterator1 first,
																   InputIterator1 last,
																   OutputIterator result,
																   Function		  fun = {}) noexcept
	{
		fun = __global_check_fun(fun);

		for (; first != last; ++first, ++result)
		{
			*result = __invoke(fun, *first);
		}

		return result;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function transform() for 仿函数 标准版 */
	template <__is_input_iterator InputIterator1, __is_output_iterator OutputIterator, class Function = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr OutputIterator
		operator()(InputIterator1 first, InputIterator1 last, OutputIterator result, Function fun = {}) noexcept
	{
		return __default_transform(__move(first), __move(last), __move(result), fun);
	}

	/* function transform() for from_容器 强化版 */
	template <__is_input_range Range, __is_output_iterator OutputIterator, class Function = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr OutputIterator operator()(Range&& rng, OutputIterator result, Function fun = {}) noexcept
	{
		return __default_transform(__begin_for_range_with_move(rng),
								   __end_for_range_with_move(rng),
								   __move(result),
								   fun);
	}

	/* function transform() for all_容器 强化版 */
	template <__is_input_range Range1, __is_output_range Range2, class Function = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(Range1&& rng1, Range2&& rng2, Function fun = {}) noexcept
	{
		return __default_transform(__begin_for_range_with_move(rng1),
								   __end_for_range_with_move(rng1),
								   __begin_for_range_with_move(rng2),
								   fun);
	}
};

constexpr inline __transform_function transform { __not_quite_object::__construct_tag {} };

/* 此处实现一些关于排序的算法 */

// 此处实现 partial_sort()
struct __partial_sort_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	static constexpr void __default_partial_sort(RandomAccessIterator first,
												 RandomAccessIterator middle,
												 RandomAccessIterator last,
												 Function			  fun  = {},
												 Projection			  proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		using value_type = __value_type_for_iter<RandomAccessIterator>;

		_RANGES make_heap(first, middle, fun, proj);

		for (RandomAccessIterator i { middle }; i != last; ++i)
		{
			if (__invoke(fun, *i, __invoke(proj, *first))) // 如果序列 2 的元素比较小
			{
				value_type value { _RANGES iter_move(i) };
				_RANGES	   _Pop_heap_hole_unchecked(first, middle, i, value, fun, proj, proj);
			}
		}

		_RANGES sort_heap(__move(first), __move(middle), fun, proj);
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function partial_sort() for 仿函数 标准版 */
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
		requires(_STD sortable<RandomAccessIterator, Function, Projection>)
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator middle,
							  RandomAccessIterator last,
							  Function			   fun	= {},
							  Projection		   proj = {}) noexcept
	{
		__default_partial_sort(__move(first), __move(middle), __move(last), fun, proj);
	}

	/* function partial_sort() for 容器、仿函数 强化版 */
	template <__is_random_access_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		requires(_STD sortable<__iterator_type_for_range<Range>, Function, Projection>)
	constexpr void operator()(Range&& rng, Function fun = {}, Projection proj = {}) noexcept
	{
		__default_partial_sort(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __partial_sort_function partial_sort { __not_quite_object::__construct_tag {} };

// 此处实现 __unguarded_insertion_sort() ，原本服务于 insertion_sort() ，单独写出是为了在其他地方调用
struct __unguarded_insertion_sort_function: private __not_quite_object
{
public:

	using __not_quite_object::__not_quite_object;

	// 插入排序 第三步（ 使用此函数需要保证 仿函数 有效 ）
	template <__is_random_access_iterator RandomAccessIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	constexpr void operator()(RandomAccessIterator last, T value, Function fun = {}, Projection proj = {}) noexcept
	{
		RandomAccessIterator next { last };
		--next;

		while (__invoke(fun, value, __invoke(proj, *next)))
		{
			*last = *next;
			last  = next;
			--next;
		}

		*last = value;
	}
};

constexpr inline __unguarded_insertion_sort_function __unguarded_insertion_sort {
	__not_quite_object::__construct_tag {}
};

// 此处实现 insertion_sort()
struct __insertion_sort_function: private __not_quite_object
{
private:

	// 插入排序 统一调用方式（这是排序的第一步）
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	static constexpr void __default_insertion_sort(RandomAccessIterator first,
												   RandomAccessIterator last,
												   Function				fun	 = {},
												   Projection			proj = {}) noexcept
	{
		if (first == last)
		{
			return;
		}
		else
		{
			using value_type = __value_type_for_iter<RandomAccessIterator>;

			// 插入排序 第二步 （ 使用此函数需要保证 仿函数 有效 ）
			for (RandomAccessIterator i { first + 1 }; i != last; ++i)
			{
				value_type value { *i };

				if (__invoke(fun, value, __invoke(proj, *first)))
				{
					// TODO: 以期实现自己的 copy_backward()
					_RANGES copy_backward(first, i, i + 1);

					*first = value;
				}
				else
				{
					// （ 单独写出是为了在其他地方调用 ）
					__unguarded_insertion_sort(i, __move(value), __global_check_fun(fun), proj);
				}
			}
		}
	}

public:

	using __not_quite_object::__not_quite_object;

	// insertion_sort() for 仿函数 标准版
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
		requires(_STD sortable<RandomAccessIterator, Function, Projection>)
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Function			   fun	= {},
							  Projection		   proj = {}) noexcept
	{
		__default_insertion_sort(__move(first), __move(last), fun, proj);
	}

	// insertion_sort()	for 容器、仿函数 强化版
	template <__is_random_access_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		requires(_STD sortable<__iterator_type_for_range<Range>, Function, Projection>)
	constexpr void operator()(Range&& rng, Function fun = {}, Projection proj = {}) noexcept
	{
		__default_insertion_sort(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __insertion_sort_function insertion_sort { __not_quite_object::__construct_tag {} };

// 此处实现 __get_median() 获取 “三点中值” 。原本服务于 quick_sort() ，单独写出是为了在其他地方调用
struct __get_median_function: private __not_quite_object
{
public:

	using __not_quite_object::__not_quite_object;

	// 函数 1 ，用于获取三个元素中的中间值（使用此函数需要保证 仿函数 有效）
	template <typename T, class Function = _RANGES less, typename Projection = _STD identity>
	constexpr const T& operator()(const T& a, const T& b, const T& c, Function fun = {}, Projection proj = {}) noexcept
	{
		if (__invoke(fun, __invoke(proj, a), __invoke(proj, b)))
		{
			if (__invoke(fun, __invoke(proj, b), __invoke(proj, c)))
			{
				return __invoke(proj, b);
			}
			else if (__invoke(fun, __invoke(proj, a), __invoke(proj, c)))
			{
				return __invoke(proj, c);
			}
			else
			{
				return __invoke(proj, a);
			}
		}
		else if (__invoke(fun, __invoke(proj, a), __invoke(proj, c)))
		{
			return __invoke(proj, a);
		}
		else if (__invoke(fun, __invoke(proj, b), __invoke(proj, c)))
		{
			return __invoke(proj, c);
		}
		else
		{
			return __invoke(proj, b);
		}
	}
};

constexpr inline __get_median_function __get_median { __not_quite_object::__construct_tag {} };

// 此处实现 __unguraded_partition() 进行无边界检查的 分割序列 。原本服务于 quick_sort() ，单独写出是为了在其他地方调用
struct __unguraded_partition_function: private __not_quite_object
{
public:

	using __not_quite_object::__not_quite_object;

	// 用于分割序列
	template <__is_random_access_iterator RandomAccessIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	constexpr RandomAccessIterator operator()(RandomAccessIterator first,
											  RandomAccessIterator last,
											  const T&			   pivot,
											  Function			   fun	= {},
											  Projection		   proj = {}) noexcept
	{
		// 分割的结果最终是：以 piovt 为节点，有如下事实
		// a、节点 pivot 处于正确的位置
		// b、节点 pivot 左边的元素均小于等于 pivot
		// c、节点 pivot 右边的节点均大于等于 pivot
		// d、返回节点 pivot 右边的第一个位置

		while (true)
		{
			while (__invoke(fun, __invoke(proj, *first), pivot))
			{
				++first;
			}

			--last;
			while (__invoke(fun, pivot, __invoke(proj, *last)))
			{
				--last;
			}

			if (!(first < last))
			{
				return first;
			}
			else
			{
				*first = __invoke(proj, *first);
				*last  = __invoke(proj, *last);

				iter_swap(first, last);

				++first;
			}
		}
	}
};

constexpr inline __unguraded_partition_function __unguraded_partition { __not_quite_object::__construct_tag {} };

// 此处实现 quick_sort()
struct __quick_sort_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	static constexpr void __default_quick_sort(RandomAccessIterator first,
											   RandomAccessIterator last,
											   Function				fun	 = {},
											   Projection			proj = {}) noexcept
	{
		if (1 < (last - first))
		{
			using value_type = __value_type_for_iter<RandomAccessIterator>;

			while (1 < (last - first))
			{
				value_type pivot { __get_median(*first, *(last - 1), *(first + (last - first) / 2), fun, proj) };

				RandomAccessIterator cut { __unguraded_partition(first, last, pivot, __global_check_fun(fun), proj) };

				if ((last - cut) < (cut - first))
				{
					__default_quick_sort(cut, last, fun, proj);
					last = cut;
				}
				else
				{
					__default_quick_sort(first, cut, fun, proj);
					first = cut;
				}
			}
		}
	}

public:

	using __not_quite_object::__not_quite_object;

	// quick_sort() for 仿函数 标准版
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
		requires(_STD sortable<RandomAccessIterator, Function, Projection>)
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator last,
							  Function			   fun	= {},
							  Projection		   proj = {}) noexcept
	{
		__default_quick_sort(__move(first), __move(last), fun, proj);
	}

	// quick_sort() for 容器、仿函数 强化版
	template <__is_random_access_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		requires(_STD sortable<__iterator_type_for_range<Range>, Function, Projection>)
	constexpr void operator()(Range&& rng, Function fun = {}, Projection proj = {}) noexcept
	{
		__default_quick_sort(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __quick_sort_function quick_sort { __not_quite_object::__construct_tag {} };

// 此处实现 merge_sort()
struct __merge_sort_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_bidirectional_iterator BidirectionalIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	static constexpr void __default_merge_sort(BidirectionalIterator first,
											   BidirectionalIterator last,
											   Function				 fun  = {},
											   Projection			 proj = {}) noexcept
	{
		using difference_type = __difference_type_for_iter<BidirectionalIterator>;
		difference_type n	  = _RANGES distance(first, last);

		if ((n == 0) || (n == 1))
		{
			return;
		}

		BidirectionalIterator mid { first + (n / 2) };

		__default_merge_sort(first, mid, fun, proj);
		__default_merge_sort(mid, last, fun, proj);

		// TODO: 以期实现自己的 inplace_merge() ，同时，此前提到插入排序的缺点之一 “借助额外内存” ，就体现在此函数中
		_RANGES inplace_merge(__move(first), __move(mid), __move(last), __global_check_fun(fun), proj);
	}

public:

	using __not_quite_object::__not_quite_object;

	/*
			* 归并排序 未被 sort 采纳，主要是考虑到：
			* 1、需要借助额外的内存
			* 2、在内存之间移动（复制）数据耗时不少
			*/

	// merge_sort() for 仿函数 强化版
	template <__is_bidirectional_iterator BidirectionalIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
		requires(_STD sortable<BidirectionalIterator, Function, Projection>)
	constexpr void operator()(BidirectionalIterator first,
							  BidirectionalIterator last,
							  Function				fun	 = {},
							  Projection			proj = {}) noexcept
	{
		__default_merge_sort(first, last, fun, proj);
	}

	// merge_sort() for 容器、仿函数 强化版
	template <__is_bidirectional_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		requires(_STD sortable<__iterator_type_for_range<Range>, Function, Projection>)
	constexpr void operator()(Range&& rng, Function fun = {}, Projection proj = {}) noexcept
	{
		__default_merge_sort(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __merge_sort_function merge_sort { __not_quite_object::__construct_tag {} };

// 此处实现 sort()
struct __sort_function: private __not_quite_object
{
private:

	// sort 辅助函数：用于控制分割恶化情况
	template <typename Size>
	static constexpr Size __get_depth_limit(Size n) noexcept // 找出 2^k <= n 的最大值 k
	{
		Size ans { 0 };

		for (; n > 1; n >>= 1)
		{
			++ans;
		}

		return ans;
	}

	// sort -- 第一部分：排序，使之 “几乎有序”
	template <__is_random_access_iterator RandomAccessIterator,
			  typename Size,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	static constexpr void __introsort_loop_sort(RandomAccessIterator first,
												RandomAccessIterator last,
												Size				 depth_limit,
												Function			 fun  = {},
												Projection			 proj = {}) noexcept
	{
		using value_type = __value_type_for_iter<RandomAccessIterator>;

		// “几乎有序” 的判断标准：需要排序的元素个数足够少，否则视为 “非 ‘几乎有序’ ”
		while (__stl_threshold < (last - first)) // “几乎有序”
		{
			if (depth_limit == 0)				 // 递归深度足够深
			{
				partial_sort(first, last, last, fun,
							 proj); // 此时调用 partial_sort()，实际上调用了一个 “堆排序”

				return;
			}

			--depth_limit;

			// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
			value_type			 pivot { __get_median(*first, *(last - 1), *(first + (last - first) / 2), fun, proj) };
			RandomAccessIterator cut { __unguraded_partition(first, last, pivot, __global_check_fun(fun), proj) };

			// 对右半段 递归sort

			// 至此，回到 while 循环，准备对左半段递归排序
			last = cut;
		}
	}

	// sort -- 第二部分：排序，使 “几乎有序” 蜕变到 “完全有序”
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	static constexpr void __final_insertion_sort(RandomAccessIterator first,
												 RandomAccessIterator last,
												 Function			  fun  = {},
												 Projection			  proj = {}) noexcept
	{
		// 待排序元素个数是否足够多？
		if (__stl_threshold < (last - first)) // 是
		{
			using value_type = __value_type_for_iter<RandomAccessIterator>;

			// 对前若干个元素 插入排序
			insertion_sort(first, first + __stl_threshold, fun, proj);

			// 对剩余元素(剩余元素数量一定少于前面的元素数量) 插入排序(无边界检查)
			for (RandomAccessIterator i { first + __stl_threshold }; i != last; ++i)
			{
				__unguarded_insertion_sort(i, __cove_type(*i, value_type), __global_check_fun(fun), proj);
			}
		}
		else // 否
		{
			// 对这些元素 插入排序
			insertion_sort(__move(first), __move(last), fun, proj);
		}
	}

	// 统一调用方式
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	static constexpr void
		__default_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}, Projection proj = {})
	{
		if (first < last) // 真实的排序由以下两个函数完成
		{
			// 排序，使之 “几乎有序”
			__introsort_loop_sort(first, last, __get_depth_limit(last - first) * 2, fun, proj);

			// 排序，使 “几乎有序” 蜕变到 “完全有序”
			__final_insertion_sort(__move(first), __move(last), fun, proj);
		}
	}

public:

	using __not_quite_object::__not_quite_object;

	// sort() for 仿函数 标准版
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
		requires(_STD sortable<RandomAccessIterator, Function, Projection>)
	constexpr inline void operator()(RandomAccessIterator first,
									 RandomAccessIterator last,
									 Function			  fun  = {},
									 Projection			  proj = {}) noexcept
	{
		__default_sort(first, last, fun, proj);
	}

	// sort() for 容器、仿函数 强化版
	template <__is_random_access_range Range, class Function = _RANGES less, typename Projection = _STD identity>
		requires(_STD sortable<__iterator_type_for_range<Range>, Function, Projection>)
	constexpr void operator()(Range&& rng, Function fun = {}, Projection proj = {}) noexcept
	{
		__default_sort(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __sort_function sort { __not_quite_object::__construct_tag {} };

// 此处实现 nth_element()
struct __nth_element_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	static constexpr void __nth_element(RandomAccessIterator first,
										RandomAccessIterator nth,
										RandomAccessIterator last,
										Function			 fun  = {},
										Projection			 proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		using value_type = __value_type_for_iter<RandomAccessIterator>;

		while (3 < (last - first)) // 如果 长度 > 3
		{
			// 采用 “三点中值”
			// 返回一个迭代器，指向分割后的右段第一个元素

			RandomAccessIterator cut { __unguraded_partition(
				first,
				last,
				__cove_type(__get_median(*first, *(last - 1), *(first + (last - first) / 2), fun, proj), value_type),
				fun,
				proj) };

			if (nth < cut)	 // 如果 指定位置 < 右段起点，（即 nth 位于右段）
			{
				first = cut; // 对右段实施分割
			}
			else			 // 否则（nth 位于左段）
			{
				last = cut;	 // 对左段实施分割
			}
		}

		__default_insertion_sort(__move(first), __move(last), fun, proj);
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function nth_element() for 仿函数 标准版 */
	template <__is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator nth,
							  RandomAccessIterator last,
							  Function			   fun	= {},
							  Projection		   proj = {}) noexcept
	{
		__nth_element(__move(first), __move(nth), __move(last), fun, proj);
	}

	/* function nth_element() for 容器、仿函数 强化版 */
	template <__is_random_access_range	  Range,
			  __is_random_access_iterator RandomAccessIterator,
			  class Function	  = _RANGES	   less,
			  typename Projection = _STD  identity>
	constexpr void operator()(Range&& rng, RandomAccessIterator nth, Function fun = {}, Projection proj = {}) noexcept
	{
		__nth_element(__begin_for_range_with_move(rng), __move(nth), __end_for_range_with_move(rng), fun, proj);
	}
};

constexpr inline __nth_element_function nth_element { __not_quite_object::__construct_tag {} };

/* 此处实现一些关于二分查找的算法 */

// 此处实现 lower_bound()
struct __upper_bound_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr ForwardIterator __default_upper_bound(ForwardIterator first,
																	  ForwardIterator last,
																	  const T&		  value,
																	  Function		  fun  = {},
																	  Projection	  proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		using difference_type = __difference_type_for_iter<ForwardIterator>;
		using iter_type_tag	  = __type_tag_for_iter<ForwardIterator>;
		constexpr bool is_random_access_iter { _STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()> };

		difference_type len { 0 };
		difference_type half { 0 };

		ForwardIterator middle { first };

		if constexpr (is_random_access_iter)
		{
			len = last - first;
		}
		else
		{
			len = _RANGES distance(first, last);
		}

		while (0 < len)
		{
			half = len >> 1;

			if constexpr (is_random_access_iter)
			{
				middle = first + half;
			}
			else
			{
				middle = first;
				_RANGES advance(middle, half);
			}

			if (__invoke(fun, value, __invoke(proj, *middle)))
			{
				len = half;
			}
			else
			{
				first = middle;
				++first;

				len -= (half + 1);
			}
		}

		return first;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* upper_bound() for 仿函数 标准版 */
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr ForwardIterator operator()(ForwardIterator first,
										 ForwardIterator last,
										 const T&		 value,
										 Function		 fun  = {},
										 Projection		 proj = {}) noexcept
	{
		return __default_upper_bound(__move(first), __move(last), __move(value), fun, proj);
	}

	/* upper_bound() for 容器、仿函数 强化版 */
	template <__is_forward_range Range, typename T, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, const T& value, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_upper_bound(__begin_for_range_with_move(rng),
									 __end_for_range_with_move(rng),
									 __move(value),
									 fun,
									 proj);
	}
};

constexpr inline __upper_bound_function upper_bound { __not_quite_object::__construct_tag {} };

// 此处实现 upper_bound()
struct __lower_bound_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr ForwardIterator __default_lower_bound(ForwardIterator first,
																	  ForwardIterator last,
																	  const T&		  value,
																	  Function		  fun  = {},
																	  Projection	  proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		using difference_type = __difference_type_for_iter<ForwardIterator>;
		using iter_type_tag	  = __type_tag_for_iter<ForwardIterator>;
		constexpr bool is_random_access_iter { _STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()> };

		difference_type len { 0 };
		difference_type half { 0 };

		ForwardIterator middle { first };

		if constexpr (is_random_access_iter)
		{
			len = last - first;
		}
		else
		{
			len = _RANGES distance(first, last);
		}

		while (0 < len)
		{
			half = len >> 1;

			if constexpr (is_random_access_iter)
			{
				middle = first + half;
			}
			else
			{
				middle = first;
				_RANGES advance(middle, half);
			}

			if (__invoke(fun, value, __invoke(proj, *middle)))
			{
				len = half;
			}
			else
			{
				first = middle;
				++first;

				len -= (half + 1);
			}
		}

		return first;
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function lower_bound() for 仿函数 标准版 */
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr ForwardIterator operator()(ForwardIterator first,
										 ForwardIterator last,
										 const T&		 value,
										 Function		 fun  = {},
										 Projection		 proj = {}) noexcept
	{
		return __default_lower_bound(__move(first), __move(last), __move(value), fun, proj);
	}

	/* function lower_bound() for 容器、仿函数 强化版 */
	template <__is_forward_range Range, typename T, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, const T& value, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_lower_bound(__begin_for_range_with_move(rng),
									 __end_for_range_with_move(rng),
									 __move(value),
									 fun,
									 proj);
	}
};

constexpr inline __lower_bound_function lower_bound { __not_quite_object::__construct_tag {} };

// 此处实现 equal_range()
struct __equal_range_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr auto __default_equal_range(ForwardIterator first,
														   ForwardIterator last,
														   const T&		   value,
														   Function		   fun	= {},
														   Projection	   proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		using difference_type = __difference_type_for_iter<ForwardIterator>;
		using iter_type_tag	  = __type_tag_for_iter<ForwardIterator>;
		constexpr bool is_random_access_iter { _STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()> };

		difference_type len { 0 };
		difference_type half { 0 };

		ForwardIterator left { first };
		ForwardIterator middle { first };
		ForwardIterator right { first };

		if constexpr (is_random_access_iter)
		{
			len = last - first;
		}
		else
		{
			len = _RANGES distance(first, last);
		}

		while (0 < len)							 // 如果整个区间尚未迭代完毕
		{
			half = len >> 1;					 // 找出中间位置

			if constexpr (is_random_access_iter) // 设定中央迭代器
			{
				middle = first + half;
			}
			else
			{
				middle = first;
				_RANGES advance(middle, half);
			}

			if (__invoke(fun, __invoke(proj, *middle), value)) // 如果 中央元素 < 指定值
			{
				first = middle;
				++first; // 将区间缩小（移至后半段），以提高效率

				len -= (half + 1);
			}
			else if (__invoke(fun, value, __invoke(proj, *middle))) // 如果 中央元素 > 指定值
			{
				len = half; // 将区间缩小（移至前半段），以提高效率
			}
			else			// 如果 中央元素 == 指定值
			{
				left  = lower_bound(first, middle, value, fun, proj); // 在前半段寻找
				right = upper_bound(__move(++middle), __move(first + len), __move(value), fun,
									proj);							  // 在后半段寻找

				return _STD pair { __move(left), __move(right) };
			}
		}

		// 整个区间内没有匹配值，则返回一对迭代器--指向第一个 大于指定值 的元素
		return _STD pair { first, first };
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function equal_range() for 仿函数 标准版 */
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(ForwardIterator first,
							  ForwardIterator last,
							  const T&		  value,
							  Function		  fun  = {},
							  Projection	  proj = {}) noexcept
	{
		return __default_equal_range(__move(first), __move(last), __move(value), fun, proj);
	}

	/* function equal_range() for 容器、仿函数 强化版 */
	template <__is_forward_range Range, typename T, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, const T& value, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_equal_range(__begin_for_range_with_move(rng),
									 __end_for_range_with_move(rng),
									 __move(value),
									 fun,
									 proj);
	}
};

constexpr inline __equal_range_function equal_range { __not_quite_object::__construct_tag {} };

// 此处实现 binary_search()
struct __binary_search_function: private __not_quite_object
{
private:

	// 统一调用方式
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD static constexpr bool __default_binary_search(ForwardIterator first,
															 ForwardIterator last,
															 const T&		 value,
															 Function		 fun  = {},
															 Projection		 proj = {}) noexcept
	{
		fun = __global_check_fun(fun);

		ForwardIterator i { lower_bound(first, last, value, fun, proj) };

		return (i != last) && !(__invoke(fun, value, __invoke(proj, *i)));
	}

public:

	using __not_quite_object::__not_quite_object;

	/* function binary_search() for 仿函数 标准版 */
	template <__is_forward_iterator ForwardIterator,
			  typename T,
			  class Function	  = _RANGES	  less,
			  typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr bool operator()(ForwardIterator first,
							  ForwardIterator last,
							  const T&		  value,
							  Function		  fun  = {},
							  Projection	  proj = {}) noexcept
	{
		return __default_binary_search(__move(first), __move(last), __move(value), fun, proj);
	}

	/* function binary_search() for 容器、仿函数 强化版 */
	template <__is_forward_range Range, typename T, class Function = _RANGES less, typename Projection = _STD identity>
	_NODISCARD_MSG("此函数的返回值不应该被忽略")
	constexpr bool operator()(Range&& rng, const T& value, Function fun = {}, Projection proj = {}) noexcept
	{
		return __default_binary_search(__begin_for_range_with_move(rng),
									   __end_for_range_with_move(rng),
									   __move(value),
									   fun,
									   proj);
	}
};

constexpr inline __binary_search_function binary_search { __not_quite_object::__construct_tag {} };


__END_NP_ZHANG

#endif // __HAS_CPP20
