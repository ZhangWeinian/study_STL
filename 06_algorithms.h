#pragma once

#include "00_basicFile.h"


#ifdef __HAS_CPP20

__BEGIN_NAMESPACE_ZHANG

// 此处实现 copy()
struct __Copy_function: private __Not_quite_object
{
private:

	/* function __copy_with_random_access_iter() -- 辅助函数 */
	template <_STD random_access_iterator RandomAccessIterator, _STD weakly_incrementable OutputIterator>
	static constexpr OutputIterator __copy_with_random_access_iter(RandomAccessIterator first,
																   RandomAccessIterator last,
																   OutputIterator		result) noexcept
	{
		using difference_type = typename _STD iter_difference_t<RandomAccessIterator>;

		for (difference_type i { last - first }; i > 0; --i, ++result, ++first) // 以 i 决定循环的次数 -- 速度快
		{
			*result = *first;
		}

		return result;
	}

	// 完全泛化版本
	template <_STD input_iterator InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  _STD weakly_incrementable		   OutputIterator>
	static constexpr OutputIterator
		__default_copy_dispatch(InputIterator first, Sentinel last, OutputIterator result) noexcept
	{
		// 此处兵分两路：根据迭代器种类的不同，调用不同的 __copy_with_iter_tag() ，为的是不同种类的迭代器所使用的循环条件不同，有快慢之分
		if constexpr (_STD random_access_iterator<InputIterator>)
		{
			// 此处单独划分出一个函数，为的是其他地方也能用到
			return __copy_with_random_access_iter(_STD move(first), _STD move(last), _STD move(result));
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

	// 偏特化版本1：第一个参数是 const Type* 指针形式，第二个参数是 Type* 指针形式
	// 为何要分为两种情况？具体实现见书 319 页。
	template <typename Type>
	static constexpr Type* __default_copy_dispatch(const Type* first, Type* last, Type* result) noexcept
	{
		using pointer_type_tag = typename __type_traits<Type>::has_trivial_assignment_operator;
		constexpr bool is_trivial_assignment_operator { _STD is_same_v<_STD true_type(), pointer_type_tag()> };

		if (is_trivial_assignment_operator) // 以下版本适用于 “指针所指之对象，具备 trivial assignment operator ”
		{
			_STD invoke(_CSTD memmove, result, first, sizeof(Type) * (last - first));

			return result + (last - first);
		}
		else // 以下版本适用于 “指针所指之对象，具备 non-trivial assignment operator ”
		{
			return __copy_with_random_access_iter(_STD move(first), _STD move(last), _STD move(result));
		}
	}

	// 偏特化版本2：两个参数都是 Type* 类型的指针
	template <typename Type>
	static constexpr Type* __default_copy_dispatch(Type* first, Type* last, Type* result) noexcept
	{
		using pointer_type_tag = typename __type_traits<Type>::has_trivial_assignment_operator;
		constexpr bool is_trivial_assignment_operator { _STD is_same_v<_STD true_type(), pointer_type_tag()> };

		if (is_trivial_assignment_operator)
		{
			_STD invoke(_CSTD memmove, result, first, sizeof(Type) * (last - first));

			return result + (last - first);
		}
		else
		{
			return __copy_with_random_access_iter(_STD move(first), _STD move(last), _STD move(result));
		}
	}

	// 统一调用方式

	/* function copy() 重载1 */
	static constexpr char*
		__default_cpoy(const char* first,
					   const char* last,
					   char* result) noexcept // 针对原生指针(可视为一种特殊的迭代器) const char* ，机型内存直接拷贝操作
	{
		_STD invoke(_CSTD memmove, result, first, last - first);

		return result + (last - first);
	}

	/* function copy() 重载2 */
	static constexpr wchar_t* __default_cpoy(
		const wchar_t* first,
		const wchar_t* last,
		wchar_t* result) noexcept // 针对原生指针（可视为一种特殊的迭代器）const wchar_t* ，执行内存直接拷贝操作
	{
		_STD invoke(_CSTD memmove, result, first, sizeof(wchar_t) * (last - first));

		return result + (last - first);
	}

	/* function copy() 一般泛型 */
	template <_STD input_iterator		InputIterator,
			  _STD weakly_incrementable OutputIterator> // 完全泛化版本
	static constexpr OutputIterator
		__default_cpoy(InputIterator first, InputIterator last, OutputIterator result) noexcept
	{
		return __default_copy_dispatch(_STD move(first), _STD move(last), _STD move(result));
	}


public:

	using __Not_quite_object::__Not_quite_object;

	/* function copy() 一般泛型 */
	template <_STD input_iterator InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  _STD weakly_incrementable		   OutputIterator>
	constexpr OutputIterator operator()(InputIterator first, Sentinel last, OutputIterator result) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		return __default_cpoy(_STD move(check_first), _STD move(check_last), _STD move(result));
	}

	/* function copy() for 容器 强化版 */
	template <_RANGES input_range Range, _STD weakly_incrementable OutputIterator>
	constexpr auto operator()(Range&& rng, OutputIterator result) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(result));
	}
};

constexpr inline __Copy_function copy { __Not_quite_object::__Construct_tag {} };

/*------------------------------------------------------------------------------------------------*/



/* 此处实现一些简单函数 */

// 给定一个初值 init ，将 [first, last) 范围内的元素依次累加到 init 上。可指定谓词 pred 与投影 proj
struct __Accumulate_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator InputIterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr Type __default_accumulate(InputIterator first,
														  InputIterator last,
														  Type			init,
														  Predicate		pred,
														  Projection	proj) noexcept
	{
		for (; first != last; ++first)
		{
			init = _STD invoke(pred, init, _STD invoke(proj, *first));
		}

		return init;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function accumulate() for 仿函数 标准版 */
	template <_STD input_iterator InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  typename Type,
			  typename Predicate  = _STD  plus<>,
			  typename Projection = _STD identity>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr Type operator()(InputIterator first, Sentinel last, Type init, Predicate pred = {}, Projection proj = {})
		const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		return __default_accumulate(_STD move(check_first),
									_STD move(check_last),
									_STD move(init),
									__check_function(pred),
									__check_function(proj));
	}

	/* function accumulate() for 容器、仿函数 强化版 */
	template <_RANGES input_range Range,
			  typename Type,
			  typename Predicate  = _STD  plus<>,
			  typename Projection = _STD identity>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, Type init, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(init), pred, proj);
	}
};

constexpr inline __Accumulate_function accumulate { __Not_quite_object::__Construct_tag {} };

// 统计 [first, last) 范围内与 value 相等的元素数量。可指定谓词 pred 与投影 proj
struct __Count_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator InputIterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr typename _STD iter_difference_t<InputIterator> __default_count(InputIterator first,
																							   InputIterator last,
																							   const Type&	 value,
																							   Predicate	 pred,
																							   Projection proj) noexcept
	{
		using difference_type = typename _STD iter_difference_t<InputIterator>;

		difference_type n = 0;
		for (; first != last; ++first)
		{
			if (_STD invoke(pred, _STD invoke(proj, *first), value))
			{
				++n;
			}
		}

		return n;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function count() for 仿函数 标准版 */
	template <_STD input_iterator InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES equal_to,
			  typename Projection = _STD   identity>
		requires(_STD indirect_binary_predicate<Predicate, _STD projected<InputIterator, Projection>, const Type*>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr typename _STD iter_difference_t<InputIterator>
		operator()(InputIterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		return __default_count(_STD move(check_first),
							   _STD move(check_last),
							   _STD move(value),
							   __check_function(pred),
							   __check_function(proj));
	}

	/* function count() for 容器、仿函数 强化版 */
	template <_RANGES input_range Range,
			  typename Type,
			  typename Predicate  = _RANGES equal_to,
			  typename Projection = _STD   identity>
		requires(_STD indirect_binary_predicate<Predicate,
												_STD projected<typename _RANGES iterator_t<Range>, Projection>,
												const Type*>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(value), pred, proj);
	}
};

constexpr inline __Count_function count { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素依次赋值为 value 之后的值。不可指定谓词 pred 与投影 proj
struct __Itoa_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Type>
	static constexpr void __default_itoa(ForwardIterator first, ForwardIterator last, Type value) noexcept
	{
		while (first != last)
		{
			*first++ = value++;
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function itoa() 标准版 */
	template <_STD forward_iterator ForwardIterator, _STD sentinel_for<ForwardIterator> Sentinel, typename Type>
	constexpr void operator()(ForwardIterator first, Sentinel last, const Type& value) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));

		__default_itoa(_STD move(check_first), _STD move(check_last), _STD move(value));
	}

	/* function itoa() for 容器 加强版 */
	template <_RANGES forward_range Range, typename Type>
	constexpr auto operator()(Range&& rng, const Type& value) const noexcept
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(value));
	}
};

constexpr inline __Itoa_function itoa { __Not_quite_object::__Construct_tag {} };

// 在 [first, last) 范围内查找第一个与 value 相等的元素。可指定谓词 pred 与投影 proj
struct __Find_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator InputIterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr InputIterator __default_find(InputIterator first,
															 InputIterator last,
															 const Type&   value,
															 Predicate	   pred,
															 Projection	   proj) noexcept
	{
		while ((first != last) && _STD invoke(pred, _STD invoke(proj, *first), value))
		{
			++first;
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function find() for 仿函数 标准版 */
	template <_STD input_iterator InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES equal_to,
			  typename Projection = _STD   identity>
		requires(_STD indirect_binary_predicate<Predicate, _STD projected<InputIterator, Projection>, const Type*>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr InputIterator
		operator()(InputIterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		return __default_find(_STD move(check_first),
							  _STD move(check_last),
							  _STD move(value),
							  __check_function(pred),
							  __check_function(proj));
	}

	/* function find() for 仿函数、容器 强化版 */
	template <_RANGES input_range Range,
			  typename Type,
			  typename Predicate  = _RANGES equal_to,
			  typename Projection = _STD   identity>
		requires(_STD indirect_binary_predicate<Predicate,
												_STD projected<typename _RANGES iterator_t<Range>, Projection>,
												const Type*>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(value), pred, proj);
	}
};

constexpr inline __Find_function find { __Not_quite_object::__Construct_tag {} };

// 在 [first1, last1) 范围内查找第一个与 [first2, last2) 范围内任意元素相等的元素。可指定谓词 pred 与投影 proj1、proj2
struct __Find_first_of_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator	InputIterator,
			  _STD forward_iterator ForwardIterator,
			  typename Predicate,
			  typename Projection1,
			  typename Projection2>
	_NODISCARD static constexpr InputIterator __default_find_first_of(InputIterator	  first1,
																	  InputIterator	  last1,
																	  ForwardIterator first2,
																	  ForwardIterator last2,
																	  Predicate		  pred,
																	  Projection1	  proj1,
																	  Projection2	  proj2) noexcept
	{
		for (; first1 != last1; ++first1)
		{
			for (ForwardIterator i = first2; i != last2; ++i)
			{
				if (_STD invoke(pred,
								_STD invoke(proj1, *first1),
								_STD invoke(proj2, *i))) // 如果序列 1 的元素与序列 2 中元素相等
				{
					return first1;
				}
			}
		}

		return last1;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function find_first_of() for 仿函数 标准版 */
	template <_STD input_iterator	InputIterator,
			  _STD forward_iterator ForwardIterator,
			  _STD sentinel_for<InputIterator> Sentinel_InputIterator,
			  _STD sentinel_for<ForwardIterator> Sentinel_ForwardIterator,
			  typename Predicate   = _RANGES	   equal_to,
			  typename Projection1 = _STD		 identity,
			  typename Projection2 = _STD		 identity>
		requires(_STD indirectly_comparable<InputIterator, ForwardIterator, Predicate, Projection1, Projection2>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr InputIterator
		operator()(InputIterator			first1,
				   Sentinel_InputIterator	last1,
				   ForwardIterator			first2,
				   Sentinel_ForwardIterator last2,
				   Predicate				pred  = {},
				   Projection1				proj1 = {},
				   Projection2				proj2 = {}) const noexcept
	{
		auto check_first1 = __unwrap_iterator<Sentinel_InputIterator>(_STD move(first1));
		auto check_first2 = __unwrap_iterator<Sentinel_ForwardIterator>(_STD move(first2));
		auto check_last1 =
			__get_last_iterator_unwrapped<InputIterator, Sentinel_InputIterator>(check_first1, _STD move(last1));
		auto check_last2 =
			__get_last_iterator_unwrapped<ForwardIterator, Sentinel_ForwardIterator>(check_first2, _STD move(last2));

		return __default_find_first_of(_STD move(check_first1),
									   _STD move(check_last1),
									   _STD move(check_first2),
									   _STD move(check_last2),
									   __check_function(pred),
									   __check_function(proj1),
									   __check_function(proj2));
	}

	/* function find_first_of() for 容器、仿函数 强化版 */
	template <_RANGES input_range		   InputRange,
			  _RANGES forward_range		   ForwardRange,
			  typename Predicate   = _RANGES equal_to,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD indirectly_comparable<typename _RANGES iterator_t<InputRange>,
											typename _RANGES iterator_t<ForwardRange>,
											Predicate,
											Projection1,
											Projection2>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto operator()(InputRange&&   rng1,
																			ForwardRange&& rng2,
																			Predicate	   pred	 = {},
																			Projection1	   proj1 = {},
																			Projection2	   proj2 = {}) const noexcept
	{
		return (
			*this)(_RANGES begin(rng1), _RANGES end(rng1), _RANGES begin(rng2), _RANGES end(rng2), pred, proj1, proj2);
	}
};

constexpr inline __Find_first_of_function find_first_of { __Not_quite_object::__Construct_tag {} };

// 交换 a 与 b 的值。不可指定谓词 pred 与投影 proj
struct __Swap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	template <typename Type>
	constexpr void operator()(Type& a, Type& b) const
		noexcept((_STD is_nothrow_move_constructible_v<Type>)&&(_STD is_nothrow_move_assignable_v<Type>))
	{
		Type tmp(__cove_type(a, Type&&));
		a = __cove_type(b, Type&&);
		b = __cove_type(tmp, Type&&);
	}
};

constexpr inline __Swap_function swap { __Not_quite_object::__Construct_tag {} };

// 交换 a 与 b 的值。不可指定谓词 pred 与投影 proj
struct __Iter_swap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	template <_STD forward_iterator ForwardIterator1, _STD forward_iterator ForwardIterator2>
	constexpr void operator()(ForwardIterator1 a, ForwardIterator2 b) const noexcept
	{
		swap(*(__cove_type(a, ForwardIterator1&&)), *(__cove_type(b, ForwardIterator2&&)));
	}
};

constexpr inline __Iter_swap_function iter_swap { __Not_quite_object::__Construct_tag {} };

// 将 [first1, last1) 范围内的元素与 [first2, first2 + (last1 - first1)) 范围内的元素互相交换。不可指定谓词 pred 与投影 proj
struct __Swap_ranges_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator1, _STD forward_iterator ForwardIterator2>
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

	using __Not_quite_object::__Not_quite_object;

	/* function swap_ranges() 标准版 */
	template <_STD forward_iterator ForwardIterator1,
			  _STD forward_iterator ForwardIterator2,
			  _STD sentinel_for<ForwardIterator1> Sentinel_ForwardIterator1>
		requires(_STD indirectly_swappable<ForwardIterator1, ForwardIterator2>)
	constexpr ForwardIterator2
		operator()(ForwardIterator1 first1, Sentinel_ForwardIterator1 last1, ForwardIterator2 first2) const noexcept
	{
		auto check_first1 = __unwrap_iterator<Sentinel_ForwardIterator1>(_STD move(first1));
		auto check_last1 =
			__get_last_iterator_unwrapped<ForwardIterator1, Sentinel_ForwardIterator1>(check_first1, _STD move(last1));

		auto check_first2 = _STD move(first2);

		return __default_swap_ranges(_STD move(check_first1), _STD move(check_last1), _STD move(check_first2));
	}

	/* function swap_ranges() for 容器 强化版 */
	template <_RANGES forward_range Range>
		requires(_STD indirectly_swappable<typename _RANGES iterator_t<Range>, typename _RANGES iterator_t<Range>>)
	constexpr auto operator()(Range&& rng1, Range&& rng2) const noexcept
	{
		return (*this)(_RANGES begin(rng1), _RANGES end(rng1), _RANGES begin(rng2));
	}
};

constexpr inline __Swap_ranges_function swap_ranges { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素依次调用 pred 。必须指定谓词 pred ，可指定投影 proj
struct __For_each_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator InputIterator,
			  typename Projection,
			  _STD indirectly_unary_invocable<_STD projected<InputIterator, Projection>> Predicate>
	static constexpr Predicate
		__default_for_each(InputIterator first, InputIterator last, Predicate pred, Projection proj) noexcept
	{
		for (; first != last; ++first)
		{
			_STD invoke(pred, _STD invoke(proj, *first));
		}

		return pred;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function for_each() 标准版 */
	template <_STD input_iterator InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  typename Projection = _STD	   identity,
			  _STD indirectly_unary_invocable<_STD projected<InputIterator, Projection>> Predicate>
		requires(requires(InputIterator iter, Predicate pred, Projection proj) {
			_STD invoke(pred, _STD invoke(proj, *iter));
		})
	constexpr Predicate
		operator()(InputIterator first, Sentinel last, Predicate pred, Projection proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		return __default_for_each(_STD move(check_first),
								  _STD move(check_last),
								  __check_function(pred),
								  __check_function(proj));
	}

	/* function for_each() for 容器 强化版 */
	template <_RANGES input_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirectly_unary_invocable<_STD projected<typename _RANGES iterator_t<Range>, Projection>> Predicate>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			_STD invoke(pred, _STD invoke(proj, *(_RANGES begin(rng))));
		})
	inline auto operator()(Range&& rng, Predicate pred, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __For_each_function for_each { __Not_quite_object::__Construct_tag {} };

// 判断 [first1, last1) 范围内的元素是否与 [first2, first2 + (last1 - first1)) 范围内的元素相等。可指定谓词 pred 与投影 proj
struct __Equal_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator InputIterator1,
			  _STD input_iterator InputIterator2,
			  typename Predicate,
			  typename Projection1,
			  typename Projection2>
	_NODISCARD static constexpr bool __default_equal(InputIterator1 first1,
													 InputIterator1 last1,
													 InputIterator2 first2,
													 InputIterator2 last2,
													 Predicate		pred,
													 Projection1	proj1,
													 Projection2	proj2) noexcept
	{
		if ((last1 - first1) != (last2 - first2)) // 如果序列 1 的元素数量与序列 2 的元素数量不相等, 返回 false
		{
			return false;
		}

		for (; first1 != last1; ++first1, ++first2) // 如果序列 1 的元素数量多于序列 2 的元素数量，顺次比较
		{
			if (!(_STD invoke(pred, _STD invoke(proj1, *first1), _STD invoke(proj2, *first2))))
			{
				return false;
			}
		}

		return true;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function equal() for 仿函数 标准版 */
	template <_STD input_iterator InputIterator1,
			  _STD input_iterator InputIterator2,
			  _STD sentinel_for<InputIterator1> Sentinel_InputIterator1,
			  _STD sentinel_for<InputIterator2> Sentinel_InputIterator2,
			  typename Predicate   = _RANGES	  equal_to,
			  typename Projection1 = _STD		identity,
			  typename Projection2 = _STD		identity>
		requires(
			requires(InputIterator1 iter1, InputIterator2 iter2, Predicate pred, Projection1 proj1, Projection2 proj2) {
				{
					_STD invoke(pred, _STD invoke(proj1, *iter1), _STD invoke(proj2, *iter2))
				} -> _STD convertible_to<bool>;
			})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr bool operator()(InputIterator1			first1,
																			Sentinel_InputIterator1 last1,
																			InputIterator2			first2,
																			Sentinel_InputIterator2 last2,
																			Predicate				pred  = {},
																			Projection1				proj1 = {},
																			Projection2 proj2 = {}) const noexcept
	{
		auto check_first1 = __unwrap_iterator<Sentinel_InputIterator1>(_STD move(first1));
		auto check_last1  = __unwrap_sentinel<InputIterator1>(_STD move(last1));

		auto check_first2 = __unwrap_iterator<Sentinel_InputIterator2>(_STD move(first2));
		auto check_last2  = __unwrap_sentinel<InputIterator2>(_STD move(last2));

		return __default_equal(_STD move(check_first1),
							   _STD move(check_last1),
							   _STD move(check_first2),
							   _STD move(check_last2),
							   __check_function(pred),
							   __check_function(proj1),
							   __check_function(proj2));
	}

	/* function equal() for 容器、仿函数 强化版 */
	template <_RANGES input_range		   Range,
			  typename Predicate   = _RANGES equal_to,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(requires(Range&& rng1, Range&& rng2, Predicate pred, Projection1 proj1, Projection2 proj2) {
			{
				_STD invoke(pred,
							_STD invoke(proj1, *(_RANGES begin(rng1))),
							_STD invoke(proj2, *(_RANGES begin(rng2))))
			} -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto operator()(Range&&		rng1,
																			Range&&		rng2,
																			Predicate	pred  = {},
																			Projection1 proj1 = {},
																			Projection2 proj2 = {}) const noexcept
	{
		return (
			*this)(_RANGES begin(rng1), _RANGES end(rng1), _RANGES begin(rng2), _RANGES end(rng2), pred, proj1, proj2);
	}
};

constexpr inline __Equal_function equal { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素依次赋值为 value 。不可指定谓词 pred 与投影 proj
struct __Fill_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Type>
	static constexpr void __default_fill(ForwardIterator first, ForwardIterator last, const Type& value) noexcept
	{
		for (; first != last; ++first)
		{
			*first = value;
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function fill() 标准版 */
	template <_STD forward_iterator ForwardIterator, _STD sentinel_for<ForwardIterator> Sentinel, typename Type>
	constexpr void operator()(ForwardIterator first, Sentinel last, const Type& value) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __unwrap_sentinel<ForwardIterator>(_STD move(last));

		__default_fill(_STD move(check_first), _STD move(check_last), _STD move(value));
	};

	/* function fill() for 容器 强化版 */
	template <_RANGES forward_range Range, typename Type>
	constexpr auto operator()(Range&& rng, const Type& value) const noexcept
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(value));
	};
};

constexpr inline __Fill_function fill { __Not_quite_object::__Construct_tag {} };

// 将 [first, first + n) 范围内的元素依次赋值为 value 。不可指定谓词 pred 与投影 proj
struct __Fill_n_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Type, _STD output_iterator<const Type&> OutputIterator>
	static constexpr OutputIterator __default_fill_n(OutputIterator first,
													 typename _STD iter_difference_t<OutputIterator> n,
													 const Type&									 value) noexcept
	{
		for (; n > 0; --n, ++first)
		{
			*first = value;
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function fill_n() 标准版 */
	template <typename Type, _STD output_iterator<const Type&> OutputIterator>
	constexpr OutputIterator operator()(OutputIterator first,
										typename _STD iter_difference_t<OutputIterator> n,
										const Type&										value) const noexcept
	{
		return __default_fill_n(_STD move(first), _STD move(n), _STD move(value));
	};

	/* function fill_n() for 容器 强化版 */
	template <typename Type, _RANGES output_range<const Type&> Range>
	constexpr auto
		operator()(Range&& rng, typename _RANGES range_difference_t<Range> n, const Type& value) const noexcept
	{
		return (*this)(_RANGES begin(rng), _STD move(n), _STD move(value));
	};
};

constexpr inline __Fill_n_function fill_n { __Not_quite_object::__Construct_tag {} };

// 返回 a 与 b 中较大的值。可指定谓词 pred 与投影 proj
struct __Max_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	/* function max() for 仿函数 标准版 */
	template <typename Type,
			  typename Projection																 = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<const Type*, Projection>> Predicate = _RANGES less>
		requires(requires(const Type& a, const Type& b, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, a), _STD invoke(proj, b))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const Type&
		operator()(const Type& a, const Type& b, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		return (_STD invoke(check_pred, _STD invoke(check_proj, a), _STD invoke(check_proj, b))) ? b : a;
	}

	/* function max() for 容器 强化版 */
	template <_RANGES input_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<typename _RANGES iterator_t<Range>, Projection>>
				  Predicate = _RANGES less>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				*(_RANGES begin(rng) + 1)
			} noexcept;

			{
				_STD invoke(pred,
							_STD invoke(proj, *(_RANGES begin(rng))),
							_STD invoke(proj, *(_RANGES begin(rng) + 1)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		auto first				= _RANGES begin(rng);
		auto last				= _RANGES end(rng);
		auto				ans = first;

		for (; first != last; ++first)
		{
			ans =
				(_STD invoke(check_pred, _STD invoke(check_proj, *first), _STD invoke(check_proj, *ans))) ? ans : first;
		}

		return *ans;
	}
};

constexpr inline __Max_function max { __Not_quite_object::__Construct_tag {} };

// 返回 a 与 b 中较小的值。可指定谓词 pred 与投影 proj
struct __Min_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	/* function min() for 仿函数 标准版 */
	template <typename Type,
			  typename Projection																 = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<const Type*, Projection>> Predicate = _RANGES greater>
		requires(requires(const Type& a, const Type& b, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, a), _STD invoke(proj, b))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const Type&
		operator()(const Type& a, const Type& b, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		return (_STD invoke(check_pred, _STD invoke(check_proj, a), _STD invoke(check_proj, b))) ? b : a;
	}

	/* function min() for 容器 强化版 */
	template <_RANGES input_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<typename _RANGES iterator_t<Range>, Projection>>
				  Predicate = _RANGES greater>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				*(_RANGES begin(rng) + 1)
			} noexcept;

			{
				_STD invoke(pred,
							_STD invoke(proj, *(_RANGES begin(rng))),
							_STD invoke(proj, *(_RANGES begin(rng) + 1)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		auto first				= _RANGES begin(rng);
		auto last				= _RANGES end(rng);
		auto				ans = first;

		for (; first != last; ++first)
		{
			ans =
				(_STD invoke(check_pred, _STD invoke(check_proj, *first), _STD invoke(check_proj, *ans))) ? ans : first;
		}

		return *ans;
	}
};

constexpr inline __Min_function min { __Not_quite_object::__Construct_tag {} };

// 返回 [first, last) 范围内的最大元素。可指定谓词 pred 与投影 proj
struct __Max_element_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Predicate, typename Projection>
	_NODISCARD static constexpr ForwardIterator
		__default_max_element(ForwardIterator first, ForwardIterator last, Predicate pred, Projection proj) noexcept
	{
		if (first == last)
		{
			return first;
		}

		ForwardIterator result { first };
		while (++first != last)
		{
			if (_STD invoke(pred, _STD invoke(proj, *result), _STD invoke(proj, *first)))
			{
				result = first;
			}
		}

		return result;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function max_element() for 仿函数 标准版 */
	template <_STD forward_iterator ForwardIterator,
			  _STD sentinel_for<ForwardIterator> Sentinel,
			  typename Projection = _STD		 identity,
			  _STD indirect_strict_weak_order<_STD projected<ForwardIterator, Projection>> Predicate = _RANGES less>
		requires(requires(ForwardIterator first, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, *first), _STD invoke(proj, *first))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr ForwardIterator
		operator()(ForwardIterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));

		return __default_max_element(_STD move(check_first),
									 _STD move(check_last),
									 __check_function(pred),
									 __check_function(proj));
	}

	/* function max_element() for 容器、仿函数 强化版 */
	template <_RANGES forward_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<typename _RANGES iterator_t<Range>, Projection>>
				  Predicate = _RANGES less>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, *(_RANGES begin(rng))), _STD invoke(proj, *(_RANGES begin(rng))))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Max_element_function max_element { __Not_quite_object::__Construct_tag {} };

// 返回 [first, last) 范围内的最小元素。可指定谓词 pred 与投影 proj
struct __Min_element_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Predicate, typename Projection>
	_NODISCARD static constexpr ForwardIterator
		__default_min_element(ForwardIterator first, ForwardIterator last, Predicate pred, Projection proj) noexcept
	{
		if (first == last)
		{
			return first;
		}

		ForwardIterator result { first };
		while (++first != last)
		{
			if (_STD invoke(pred, _STD invoke(proj, *result), _STD invoke(proj, *first)))
			{
				result = first;
			}
		}

		return result;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function min_element() for 仿函数 标准版 */
	template <_STD forward_iterator ForwardIterator,
			  _STD sentinel_for<ForwardIterator> Sentinel,
			  typename Projection = _STD		 identity,
			  _STD indirect_strict_weak_order<_STD projected<ForwardIterator, Projection>> Predicate = _RANGES greater>
		requires(requires(ForwardIterator first, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, *first), _STD invoke(proj, *first))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr ForwardIterator
		operator()(ForwardIterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));

		return __default_min_element(_STD move(check_first),
									 _STD move(check_last),
									 __check_function(pred),
									 __check_function(proj));
	}

	/* function min_element() for 容器、仿函数 强化版 */
	template <_RANGES forward_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<typename _RANGES iterator_t<Range>, Projection>>
				  Predicate = _RANGES greater>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, *(_RANGES begin(rng))), _STD invoke(proj, *(_RANGES begin(rng))))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Min_element_function min_element { __Not_quite_object::__Construct_tag {} };

// 将 [first1, last1) 与 [first2, last2) 保持有序合并到 result 中。可指定谓词 pred 与投影 proj1、proj2
struct __Merge_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator InputIterator1,
			  _STD input_iterator InputIterator2,
			  _STD output_iterator<const _STD iter_value_t<InputIterator1>&> OutputIterator,
			  typename Predicate,
			  typename Projection1,
			  typename Projection2>
	_NODISCARD static constexpr OutputIterator __default_merge(InputIterator1 first1,
															   InputIterator1 last1,
															   InputIterator2 first2,
															   InputIterator2 last2,
															   OutputIterator result,
															   Predicate	  pred,
															   Projection1	  proj1,
															   Projection2	  proj2) noexcept
	{
		while ((first1 != last1) && (first2 != last2)) // 若两个序列都未完成，则继续
		{
			if (_STD invoke(pred, _STD invoke(proj2, *first2), _STD invoke(proj1, *first1))) // 若序列 2 的元素比较小
			{
				*result = *first2; // 则记录序列 2 的元素
				++first2;		   // 同时序列 2 前进 1 位
			}
			else				   // 反之同理
			{
				*result = *first1;
				++first1;
			}

			++result; // 每记录一个元素，result 前进 1 位
		}

		// 最后剩余元素拷贝到目的端。（以下两个序列一定至少有一个为空）
		return copy(_STD move(first2), _STD move(last2), copy(_STD move(first1), _STD move(last1), _STD move(result)));
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function merge() for 仿函数 标准版 */
	template <_STD input_iterator InputIterator1,
			  _STD input_iterator InputIterator2,
			  _STD sentinel_for<InputIterator1> Sentinel_InputIterator1,
			  _STD sentinel_for<InputIterator2> Sentinel_InputIterator2,
			  _STD output_iterator<const _STD iter_value_t<InputIterator1>&> OutputIterator,
			  typename Predicate   = _RANGES								   less,
			  typename Projection1 = _STD									 identity,
			  typename Projection2 = _STD									 identity>
		requires(_STD mergeable<InputIterator1, InputIterator2, OutputIterator, Predicate, Projection1, Projection2>)
	constexpr OutputIterator operator()(InputIterator1			first1,
										Sentinel_InputIterator1 last1,
										InputIterator2			first2,
										Sentinel_InputIterator2 last2,
										OutputIterator			result,
										Predicate				pred  = {},
										Projection1				proj1 = {},
										Projection2				proj2 = {}) const noexcept
	{
		auto check_first1 = __unwrap_iterator<Sentinel_InputIterator1>(_STD move(first1));
		auto check_first2 = __unwrap_iterator<Sentinel_InputIterator2>(_STD move(first2));
		auto check_last1 =
			__get_last_iterator_unwrapped<InputIterator1, Sentinel_InputIterator1>(check_first1, _STD move(last1));
		auto check_last2 =
			__get_last_iterator_unwrapped<InputIterator2, Sentinel_InputIterator2>(check_first2, _STD move(last2));

		return __default_merge(_STD move(check_first1),
							   _STD move(check_last1),
							   _STD move(check_first2),
							   _STD move(check_last2),
							   _STD move(result),
							   __check_function(pred),
							   __check_function(proj1),
							   __check_function(proj2));
	}

	/* function merge() for 容器、仿函数 强化版 */
	template <_RANGES input_range InputRange1,
			  _RANGES input_range InputRange2,
			  _STD output_iterator<const _RANGES range_value_t<InputRange1>&> OutputIterator,
			  typename Predicate   = _RANGES									less,
			  typename Projection1 = _STD									  identity,
			  typename Projection2 = _STD									  identity>
		requires(_STD mergeable<typename _RANGES iterator_t<InputRange1>,
								typename _RANGES iterator_t<InputRange2>,
								OutputIterator,
								Predicate,
								Projection1,
								Projection2>)
	constexpr auto operator()(InputRange1&&	 rng1,
							  InputRange2&&	 rng2,
							  OutputIterator result,
							  Predicate		 pred  = {},
							  Projection1	 proj1 = {},
							  Projection2	 proj2 = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng1),
					   _RANGES end(rng1),
					   _RANGES begin(rng2),
					   _RANGES end(rng2),
					   _STD	   move(result),
					   pred,
					   proj1,
					   proj2);
	}
};

constexpr inline __Merge_function merge { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素依次调用 pred ，并将结果保存到 result 中。可指定谓词 pred 与投影 proj
struct __Transform_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD input_iterator InputIterator,
			  _STD output_iterator<const _STD iter_value_t<InputIterator>&> OutputIterator,
			  typename Predicate,
			  typename Projection>
	_NODISCARD static constexpr OutputIterator __default_transform(InputIterator  first,
																   InputIterator  last,
																   OutputIterator result,
																   Predicate	  pred,
																   Projection	  proj) noexcept
	{
		for (; first != last; ++first, ++result)
		{
			*result = _STD invoke(pred, _STD invoke(proj, *first));
		}

		return result;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function transform() for 仿函数 标准版 */
	template <_STD input_iterator InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  _STD output_iterator<const _STD iter_value_t<InputIterator>&> OutputIterator,
			  typename Predicate  = _STD									 identity,
			  typename Projection = _STD									identity>
		requires(
			_STD indirectly_writable<OutputIterator,
									 _STD indirect_result_t<Predicate&, _STD projected<InputIterator, Projection>>>)
	constexpr OutputIterator operator()(InputIterator  first,
										Sentinel	   last,
										OutputIterator result,
										Predicate	   pred = {},
										Projection	   proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		return __default_transform(_STD move(check_first),
								   _STD move(check_last),
								   _STD move(result),
								   __check_function(pred),
								   __check_function(proj));
	}

	/* function transform() for 容器 强化版 */
	template <_RANGES input_range InputRange,
			  _STD output_iterator<const _RANGES range_value_t<InputRange>&> OutputIterator,
			  typename Predicate  = _STD									  identity,
			  typename Projection = _STD									 identity>
		requires(_STD indirectly_writable<
				 OutputIterator,
				 _STD indirect_result_t<Predicate&, _STD projected<_RANGES iterator_t<InputRange>, Projection>>>)
	constexpr auto
		operator()(InputRange&& rng1, OutputIterator result, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng1), _RANGES end(rng1), _STD move(result), pred, proj);
	}
};

constexpr inline __Transform_function transform { __Not_quite_object::__Construct_tag {} };

/*------------------------------------------------------------------------------------------------*/



/* 此处实现一些关于排序的算法 */

template <typename Iterator, typename Predicate, typename Projection>
concept __basic_concept_for_sort_function = _STD sortable<Iterator, Predicate, Projection>;

// 将 [first, last) 范围内的元素部分排序，使得 [first, middle) 范围内的元素为最小的 middle - first 个元素。可指定谓词 pred 与投影 proj
struct __Partial_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD random_access_iterator RandomAccessIterator, typename Predicate, typename Projection>
	static constexpr void __default_partial_sort(RandomAccessIterator first,
												 RandomAccessIterator middle,
												 RandomAccessIterator last,
												 Predicate			  pred,
												 Projection			  proj) noexcept
	{
		using value_type = typename _STD iter_value_t<RandomAccessIterator>;

		_RANGES make_heap(first, middle, pred, proj);

		for (RandomAccessIterator i { middle }; i != last; ++i)
		{
			if (_STD invoke(pred, *i, _STD invoke(proj, *first))) // 如果序列 2 的元素比较小
			{
				value_type value { _RANGES iter_move(i) };
				_RANGES	   _Pop_heap_hole_unchecked(first, middle, i, value, pred, proj, proj);
			}
		}

		_RANGES sort_heap(_STD move(first), _STD move(middle), pred, proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function partial_sort() for 仿函数 标准版 */
	template <_STD random_access_iterator RandomAccessIterator,
			  _STD sentinel_for<RandomAccessIterator> Sentinel,
			  typename Predicate  = _RANGES			   less,
			  typename Projection = _STD			  identity>
		requires(__basic_concept_for_sort_function<RandomAccessIterator, Predicate, Projection>)
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator middle,
							  Sentinel			   last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const noexcept
	{
		auto check_first  = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_middle = __unwrap_iterator<Sentinel>(_STD move(middle));
		auto check_last	  = __get_last_iterator_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__default_partial_sort(_STD move(check_first),
							   _STD move(check_middle),
							   _STD move(check_last),
							   __check_function(pred),
							   __check_function(proj));
	}

	/* function partial_sort() for 容器、仿函数 强化版 */
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<typename _RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr auto operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Partial_sort_function partial_sort { __Not_quite_object::__Construct_tag {} };

// 此处实现 __zh_Unguarded_insertion_sort() ，原本服务于 insertion_sort() ，单独写出是为了在其他地方调用
struct __zh_Unguarded_insertion_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD random_access_iterator RandomAccessIterator, typename Type, typename Predicate, typename Projection>
	static constexpr void __default_unguarded_insertion_sort(RandomAccessIterator last,
															 Type				  value,
															 Predicate			  pred,
															 Projection			  proj) noexcept
	{
		RandomAccessIterator next { last };
		--next;

		while (_STD invoke(pred, value, _STD invoke(proj, *next)))
		{
			*last = *next;
			last  = next;
			--next;
		}

		*last = value;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// 插入排序 第三步（ 使用此函数需要保证 仿函数 有效 ）
	template <_STD random_access_iterator RandomAccessIterator,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr void
		operator()(RandomAccessIterator last, Type value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		__default_unguarded_insertion_sort(_STD move(last),
										   _STD move(value),
										   __check_function(pred),
										   __check_function(proj));
	}
};

constexpr inline __zh_Unguarded_insertion_sort_function __zh_Unguarded_insertion_sort {
	__Not_quite_object::__Construct_tag {}
};

// 将 [first, last) 范围内的元素插入排序。可指定谓词 pred 与投影 proj
struct __Insertion_sort_function: private __Not_quite_object
{
private:

	// 插入排序 统一调用方式（这是排序的第一步）
	template <_STD random_access_iterator RandomAccessIterator, typename Predicate, typename Projection>
	static constexpr void __default_insertion_sort(RandomAccessIterator first,
												   RandomAccessIterator last,
												   Predicate			pred,
												   Projection			proj) noexcept
	{
		if (first == last)
		{
			return;
		}
		else
		{
			using value_type = typename _STD iter_value_t<RandomAccessIterator>;

			value_type value { *first };

			// 插入排序 第二步 （ 使用此函数需要保证 仿函数 有效 ）
			for (RandomAccessIterator i { first + 1 }; i != last; ++i)
			{
				value = *i;

				if (_STD invoke(pred, value, _STD invoke(proj, *first)))
				{
					// TODO: 以期实现自己的 copy_backward()
					_RANGES copy_backward(first, i, i + 1);

					*first = value;
				}
				else
				{
					// （ 单独写出是为了在其他地方调用 ）
					__zh_Unguarded_insertion_sort(i, _STD move(value), pred, proj);
				}
			}
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// insertion_sort() for 仿函数 标准版
	template <_STD random_access_iterator RandomAccessIterator,
			  _STD sentinel_for<RandomAccessIterator> Sentinel,
			  typename Predicate  = _RANGES			   less,
			  typename Projection = _STD			  identity>
		requires(__basic_concept_for_sort_function<RandomAccessIterator, Predicate, Projection>)
	constexpr void
		operator()(RandomAccessIterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__default_insertion_sort(_STD move(check_first),
								 _STD move(check_last),
								 __check_function(pred),
								 __check_function(proj));
	}

	// insertion_sort()	for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<typename _RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr auto operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Insertion_sort_function insertion_sort { __Not_quite_object::__Construct_tag {} };

// 此处实现 __zh_Get_median_of_three() 获取 “三点中值” 。原本服务于 quick_sort() ，单独写出是为了在其他地方调用
struct __zh_Get_median_of_three_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	// 函数 1 ，用于获取三个元素中的中间值（使用此函数需要保证 仿函数 有效）
	template <typename Type, typename Predicate, typename Projection>
	constexpr const Type&
		operator()(const Type& a, const Type& b, const Type& c, Predicate pred, Projection proj) const noexcept
	{
		auto check_a = _STD invoke(proj, a);
		auto check_b = _STD invoke(proj, b);
		auto check_c = _STD invoke(proj, c);

		if (_STD invoke(pred, check_a, check_b))
		{
			if (_STD invoke(pred, check_b, check_c))
			{
				return b;
			}
			else if (_STD invoke(pred, check_a, check_c))
			{
				return c;
			}
			else
			{
				return a;
			}
		}
		else if (_STD invoke(pred, check_a, check_c))
		{
			return a;
		}
		else if (_STD invoke(pred, check_b, check_c))
		{
			return c;
		}
		else
		{
			return b;
		}
	}
};

constexpr inline __zh_Get_median_of_three_function __zh_Get_median_of_three { __Not_quite_object::__Construct_tag {} };

// 此处实现 __zh_Unguraded_partition() 进行无边界检查的 分割序列 。原本服务于 quick_sort() ，单独写出是为了在其他地方调用
struct __zh_Unguraded_partition_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD random_access_iterator RandomAccessIterator, typename Type, typename Predicate, typename Projection>
	static constexpr RandomAccessIterator __default_unguraded_partition(RandomAccessIterator first,
																		RandomAccessIterator last,
																		const Type&			 pivot,
																		Predicate			 pred,
																		Projection			 proj) noexcept
	{
		while (true)
		{
			while (_STD invoke(pred, _STD invoke(proj, *first), pivot))
			{
				++first;
			}

			--last;
			while (_STD invoke(pred, pivot, _STD invoke(proj, *last)))
			{
				--last;
			}

			if (!(first < last))
			{
				return first;
			}
			else
			{
				iter_swap(first, last);
				++first;
			}
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// 用于分割序列
	template <_STD random_access_iterator RandomAccessIterator,
			  _STD sentinel_for<RandomAccessIterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr RandomAccessIterator operator()(RandomAccessIterator first,
											  Sentinel			   last,
											  const Type&		   pivot,
											  Predicate			   pred = {},
											  Projection		   proj = {}) const noexcept
	{
		// 分割的结果最终是：以 piovt 为节点，有如下事实
		// a、节点 pivot 处于正确的位置
		// b、节点 pivot 左边的元素均小于等于 pivot
		// c、节点 pivot 右边的节点均大于等于 pivot
		// d、返回节点 pivot 右边的第一个位置

		return __default_unguraded_partition(_STD move(first),
											 _STD move(last),
											 _STD move(pivot),
											 __check_function(pred),
											 __check_function(proj));
	}
};

constexpr inline __zh_Unguraded_partition_function __zh_Unguraded_partition { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素快速排序。可指定谓词 pred 与投影 proj
struct __Quick_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD random_access_iterator RandomAccessIterator, typename Predicate, typename Projection>
	static constexpr void __default_quick_sort(RandomAccessIterator first,
											   RandomAccessIterator last,
											   Predicate			pred,
											   Projection			proj) noexcept
	{
		using value_type = typename _STD iter_value_t<RandomAccessIterator>;

		while (1 < (last - first))
		{
			value_type pivot {
				__zh_Get_median_of_three(*first, *(last - 1), *(first + (last - first) / 2), pred, proj)
			};

			RandomAccessIterator cut { __zh_Unguraded_partition_function {
				__Not_quite_object::__Construct_tag {} }(first, last, _STD move(pivot), pred, proj) };

			if ((last - cut) < (cut - first))
			{
				__default_quick_sort(cut, last, pred, proj);
				last = cut;
			}
			else
			{
				__default_quick_sort(first, cut, pred, proj);
				first = cut;
			}
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// quick_sort() for 仿函数 标准版
	template <_STD random_access_iterator RandomAccessIterator,
			  _STD sentinel_for<RandomAccessIterator> Sentinel,
			  typename Predicate  = _RANGES			   less,
			  typename Projection = _STD			  identity>
		requires(__basic_concept_for_sort_function<RandomAccessIterator, Predicate, Projection>)
	constexpr RandomAccessIterator
		operator()(RandomAccessIterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ans = _RANGES next(first, last);

		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__default_quick_sort(_STD move(check_first),
							 _STD move(check_last),
							 __check_function(pred),
							 __check_function(proj));

		return ans;
	}

	// quick_sort() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<typename _RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr auto operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Quick_sort_function quick_sort { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素归并排序。可指定谓词 pred 与投影 proj
struct __Merge_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD bidirectional_iterator BidirectionalIterator, typename Predicate, typename Projection>
	static constexpr void __default_merge_sort(BidirectionalIterator first,
											   BidirectionalIterator last,
											   Predicate			 pred,
											   Projection			 proj) noexcept
	{
		using difference_type = typename _STD iter_difference_t<BidirectionalIterator>;
		difference_type n	  = _RANGES distance(first, last);

		if ((n == 0) || (n == 1))
		{
			return;
		}

		BidirectionalIterator mid { first + (n / 2) };

		__default_merge_sort(first, mid, pred, proj);
		__default_merge_sort(mid, last, pred, proj);

		// TODO: 以期实现自己的 inplace_merge() ，同时，此前提到插入排序的缺点之一 “借助额外内存” ，就体现在此函数中
		_RANGES inplace_merge(_STD move(first), _STD move(mid), _STD move(last), pred, proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/*
			* 归并排序 未被 sort 采纳，主要是考虑到：
			* 1、需要借助额外的内存
			* 2、在内存之间移动（复制）数据耗时不少
			*/

	// merge_sort() for 仿函数 强化版
	template <_STD bidirectional_iterator BidirectionalIterator,
			  _STD sentinel_for<BidirectionalIterator> Sentinel,
			  typename Predicate  = _RANGES				less,
			  typename Projection = _STD			   identity>
		requires(__basic_concept_for_sort_function<BidirectionalIterator, Predicate, Projection>)
	constexpr BidirectionalIterator
		operator()(BidirectionalIterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ans = _RANGES next(first, last);

		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<BidirectionalIterator, Sentinel>(check_first, _STD move(last));

		__default_merge_sort(_STD move(check_first),
							 _STD move(check_last),
							 __check_function(pred),
							 __check_function(proj));

		return ans;
	}

	// merge_sort() for 容器、仿函数 强化版
	template <_RANGES bidirectional_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<typename _RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr auto operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Merge_sort_function merge_sort { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素排序。可指定谓词 pred 与投影 proj
struct __Sort_function: private __Not_quite_object
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
	template <_STD random_access_iterator RandomAccessIterator, typename Size, typename Predicate, typename Projection>
	static constexpr void __introsort_loop_sort(RandomAccessIterator first,
												RandomAccessIterator last,
												Size				 depth_limit,
												Predicate			 pred,
												Projection			 proj) noexcept
	{
		using value_type	  = typename _STD	   iter_value_t<RandomAccessIterator>;
		using difference_type = typename _STD iter_difference_t<RandomAccessIterator>;

		// “几乎有序” 的判断标准：需要排序的元素个数足够少，否则视为 “非 ‘几乎有序’ ”
		while ((__stl_threshold<difference_type>) < (last - first)) // “几乎有序”
		{
			if (depth_limit == 0)									// 递归深度足够深
			{
				partial_sort(first, last, last, pred,
							 proj); // 此时调用 partial_sort()，实际上调用了一个 “堆排序”

				return;
			}

			--depth_limit;

			// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割

			value_type pivot {
				__zh_Get_median_of_three(*first, *(last - 1), *(first + (last - first) / 2), pred, proj)
			};

			RandomAccessIterator cut { __zh_Unguraded_partition(first, last, _STD move(pivot), pred, proj) };

			// 对右半段 递归sort
			__introsort_loop_sort(cut, last, depth_limit, pred, proj);

			// 至此，回到 while 循环，准备对左半段递归排序
			last = cut;
		}
	}

	// sort -- 第二部分：排序，使 “几乎有序” 蜕变到 “完全有序”
	template <_STD random_access_iterator RandomAccessIterator, typename Predicate, typename Projection>
	static constexpr void __final_insertion_sort(RandomAccessIterator first,
												 RandomAccessIterator last,
												 Predicate			  pred,
												 Projection			  proj) noexcept
	{
		using value_type	  = typename _STD	   iter_value_t<RandomAccessIterator>;
		using difference_type = typename _STD iter_difference_t<RandomAccessIterator>;

		// 待排序元素个数是否足够多？
		if ((__stl_threshold<difference_type>) < (last - first)) // 是
		{
			// 对前若干个元素 插入排序
			insertion_sort(first, first + (__stl_threshold<difference_type>), pred, proj);

			// 对剩余元素(剩余元素数量一定少于前面的元素数量) 插入排序(无边界检查)
			for (RandomAccessIterator i { first + (__stl_threshold<difference_type>)}; i != last; ++i)
			{
				__zh_Unguarded_insertion_sort(i, __cove_type(*i, value_type), pred, proj);
			}
		}
		else // 否
		{
			// 对这些元素 插入排序
			insertion_sort(_STD move(first), _STD move(last), pred, proj);
		}
	}

	// 统一调用方式
	template <_STD random_access_iterator RandomAccessIterator, typename Predicate, typename Projection>
	static constexpr void
		__default_sort(RandomAccessIterator first, RandomAccessIterator last, Predicate pred, Projection proj)
	{
		if (first < last) // 真实的排序由以下两个函数完成
		{
			// 排序，使之 “几乎有序”
			__introsort_loop_sort(
				first,
				last,
				(__get_depth_limit<typename _STD iter_difference_t<RandomAccessIterator>>(last - first)) * 2,
				pred,
				proj);

			// 排序，使 “几乎有序” 蜕变到 “完全有序”
			__final_insertion_sort(_STD move(first), _STD move(last), pred, proj);
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// sort() for 仿函数 标准版
	template <_STD random_access_iterator RandomAccessIterator,
			  _STD sentinel_for<RandomAccessIterator> Sentinel,
			  typename Predicate  = _RANGES			   less,
			  typename Projection = _STD			  identity>
		requires(__basic_concept_for_sort_function<RandomAccessIterator, Predicate, Projection>)
	constexpr inline RandomAccessIterator
		operator()(RandomAccessIterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ans = _RANGES next(first, last);

		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__default_sort(_STD move(check_first), _STD move(check_last), __check_function(pred), __check_function(proj));

		return ans;
	}

	// sort() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<typename _RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr auto operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Sort_function sort { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素进行分割，使得 [first, nth) 范围内的元素均小于等于 [nth, last) 范围内的元素。可指定谓词 pred 与投影 proj
struct __Nth_element_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD random_access_iterator RandomAccessIterator, typename Predicate, typename Projection>
	static constexpr void __nth_element(RandomAccessIterator first,
										RandomAccessIterator nth,
										RandomAccessIterator last,
										Predicate			 pred,
										Projection			 proj) noexcept
	{
		using value_type = typename _STD iter_value_t<RandomAccessIterator>;

		while (3 < (last - first)) // 如果 长度 > 3
		{
			// 采用 “三点中值”
			// 返回一个迭代器，指向分割后的右段第一个元素

			value_type pivot {
				__zh_Get_median_of_three(*first, *(last - 1), *(first + (last - first) / 2), pred, proj)
			};

			RandomAccessIterator cut { __zh_Unguraded_partition(first, last, pivot, pred, proj) };

			if (nth < cut)	 // 如果 指定位置 < 右段起点，（即 nth 位于右段）
			{
				first = cut; // 对右段实施分割
			}
			else			 // 否则（nth 位于左段）
			{
				last = cut;	 // 对左段实施分割
			}
		}

		insertion_sort(_STD move(first), _STD move(last), pred, proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function nth_element() for 仿函数 标准版 */
	template <_STD random_access_iterator RandomAccessIterator,
			  _STD sentinel_for<RandomAccessIterator> Sentinel,
			  typename Predicate  = _RANGES			   less,
			  typename Projection = _STD			  identity>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator nth,
							  Sentinel			   last,
							  Predicate			   pred = {},
							  Projection		   proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_nth	 = __unwrap_iterator<Sentinel>(_STD move(nth));
		auto check_last	 = __get_last_iterator_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__nth_element(_STD move(check_first),
					  _STD move(check_nth),
					  _STD move(check_last),
					  __check_function(pred),
					  __check_function(proj));
	}

	/* function nth_element() for 容器、仿函数 强化版 */
	template <_RANGES random_access_range  Range,
			  _STD random_access_iterator  RandomAccessIterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr auto
		operator()(Range&& rng, RandomAccessIterator nth, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		(*this)(_RANGES begin(rng), _STD move(nth), _RANGES end(rng), pred, proj);
	}
};

constexpr inline __Nth_element_function nth_element { __Not_quite_object::__Construct_tag {} };

/*------------------------------------------------------------------------------------------------*/



/* 此处实现一些关于二分查找的算法 */

template <typename ForwardIterator, typename Type, typename Predicate, typename Projection>
concept __basic_concept_for_binary_search_function =
	requires(ForwardIterator first, ForwardIterator last, const Type& value, Predicate pred, Projection proj) {
		{
			_STD invoke(pred, _STD invoke(proj, *first), value)
		} noexcept -> _STD convertible_to<bool>;

		{
			_STD invoke(pred, value, _STD invoke(proj, *first))
		} noexcept -> _STD convertible_to<bool>;

		{
			_STD invoke(pred, _STD invoke(proj, *first), _STD invoke(proj, *first))
		} noexcept -> _STD convertible_to<bool>;
	};

template <typename Result, typename Wrapped, typename Unwrapped>
_NODISCARD constexpr Result __rewrap_subrange(Wrapped& value, _RANGES subrange<Unwrapped>&& result)
{
	// conditionally computes a wrapped subrange from a wrapped iterator or range and unwrapped subrange
	if constexpr (_STD is_same_v<Result, _RANGES dangling>)
	{
		return _RANGES dangling {};
	}
	else if constexpr (_STD is_same_v<Result, _RANGES subrange<Unwrapped>>)
	{
		return _STD move(result);
	}
	else if constexpr (_RANGES range<Wrapped>)
	{
		auto first				  = _RANGES begin(value);
		auto				 last = first;

		first._Seek_to(result.begin());
		last._Seek_to(result.end());

		return Result { _STD move(first), _STD move(last) };
	}
	else
	{
		auto last = value;

		value._Seek_to(result.begin());
		last._Seek_to(result.end());

		return Result { _STD move(value), _STD move(last) };
	}
}

template <_RANGES forward_range Range, class Iterator>
_NODISCARD constexpr typename _RANGES iterator_t<Range> __rewrap_iterator(Range&& rng, Iterator&& iterator)
{
	if constexpr (_STD is_same_v<_STD remove_cvref_t<Iterator>, typename _RANGES iterator_t<Range>>)
	{
		return _STD forward<Iterator>(iterator);
	}
	else
	{
		auto result = _RANGES begin(rng);
		result._Seek_to(_STD forward<Iterator>(iterator));
		return result;
	}
}

template <typename Iterator, typename UIterator>
concept __wrapped_seekable =
	requires(Iterator iterator, UIterator uiterator) { iterator._Seek_to(_STD forward<UIterator>(uiterator)); };

template <typename Iterator, typename UIterator>
constexpr void __seek_wrapped(Iterator& iterator, UIterator&& uiterauor)
{
	if constexpr (__wrapped_seekable<Iterator, UIterator>)
	{
		iterator._Seek_to(_STD forward<UIterator>(uiterauor));
	}
	else
	{
		iterator = _STD forward<UIterator>(uiterauor);
	}
}

// 返回一个有序序列中第一个小于等于给定值的位置。可指定谓词 pred 与投影 proj
struct __Lower_bound_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr ForwardIterator
		__default_lower_bound(ForwardIterator first,
							  typename _STD iter_difference_t<ForwardIterator> lenth,
							  const Type&									   value,
							  Predicate										   pred,
							  Projection									   proj) noexcept
	{
		using difference_type = typename _STD iter_difference_t<ForwardIterator>;

		difference_type half { 0 };

		ForwardIterator middle { first };

		while (0 < lenth)
		{
			half = lenth >> 1;

			middle = _RANGES next(first, half);

			if (_STD invoke(pred, _STD invoke(proj, *middle), value))
			{
				first = _STD move(middle);
				++first;

				lenth -= __cove_type((half + 1), difference_type);
			}
			else
			{
				lenth = half;
			}
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function lower_bound() for 仿函数 标准版 */
	template <_STD forward_iterator ForwardIterator,
			  _STD sentinel_for<ForwardIterator> Sentinel,
			  typename Type,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<const Type*, _STD projected<ForwardIterator, Projection>> Predicate =
				  _RANGES																				less>
		requires(__basic_concept_for_binary_search_function<ForwardIterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr ForwardIterator
		operator()(ForwardIterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		auto	   check_first = __unwrap_iterator<Sentinel>(first);
		const auto lenth = _RANGES distance(check_first, __unwrap_sentinel<ForwardIterator, Sentinel>(_STD move(last)));

		check_first = __default_lower_bound(_STD move(check_first),
											_STD move(lenth),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		__seek_wrapped(first, _STD move(check_first));

		return first;
	}

	/* function lower_bound() for 容器、仿函数 强化版 */
	template <
		_RANGES forward_range Range,
		typename Type,
		typename Projection = _STD identity,
		_STD indirect_strict_weak_order<const Type*, _STD projected<typename _RANGES iterator_t<Range>, Projection>>
			Predicate = _RANGES less>
		requires(
			__basic_concept_for_binary_search_function<typename _RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		const auto lenth = _RANGES distance(rng);
		auto result		 = __default_lower_bound(_RANGES begin(rng), _STD move(lenth), _STD move(value), pred, proj);

		return __rewrap_iterator(rng, _STD move(result));
	}
};

constexpr inline __Lower_bound_function lower_bound { __Not_quite_object::__Construct_tag {} };

// 返回一个有序序列中第一个大于等于给定值的位置。可指定谓词 pred 与投影 proj
struct __Upper_bound_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr ForwardIterator
		__default_upper_bound(ForwardIterator first,
							  typename _STD iter_difference_t<ForwardIterator> lenth,
							  const Type&									   value,
							  Predicate										   pred,
							  Projection									   proj) noexcept
	{
		using difference_type = typename _STD iter_difference_t<ForwardIterator>;

		difference_type half { 0 };

		ForwardIterator middle { first };

		while (0 < lenth)
		{
			half = lenth >> 1;

			middle = _RANGES next(first, half);

			if (_STD invoke(pred, value, _STD invoke(proj, *middle)))
			{
				lenth = half;
			}
			else
			{
				first = _STD move(middle);
				++first;

				lenth -= __cove_type((half + 1), difference_type);
			}
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* upper_bound() for 仿函数 标准版 */
	template <_STD forward_iterator ForwardIterator,
			  _STD sentinel_for<ForwardIterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<ForwardIterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr ForwardIterator
		operator()(ForwardIterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		auto	   check_first = __unwrap_iterator<Sentinel>(first);
		const auto lenth = _RANGES distance(check_first, __unwrap_sentinel<ForwardIterator, Sentinel>(_STD move(last)));

		check_first = __default_upper_bound(_STD move(check_first),
											_STD move(lenth),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		__seek_wrapped(first, _STD move(check_first));

		return first;
	}

	/* upper_bound() for 容器、仿函数 强化版 */
	template <_RANGES forward_range Range,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(
			__basic_concept_for_binary_search_function<typename _RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		const auto lenth = _RANGES distance(rng);
		auto result		 = __default_upper_bound(_RANGES begin(rng), _STD move(lenth), _STD move(value), pred, proj);

		return __rewrap_iterator(rng, _STD move(result));
	}
};

constexpr inline __Upper_bound_function upper_bound { __Not_quite_object::__Construct_tag {} };

// 返回一个有序序列中与给定值相同的 begin 位置和 end 位置。可指定谓词 pred 与投影 proj
struct __Equal_range_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr _RANGES subrange<ForwardIterator>
										__default_equal_range(ForwardIterator first,
															  typename _STD iter_difference_t<ForwardIterator> lenth,
															  const Type&									   value,
															  Predicate										   pred,
															  Projection									   proj) noexcept
	{
		using difference_type = typename _STD iter_difference_t<ForwardIterator>;

		difference_type half { 0 };

		ForwardIterator middle { first };

		while (0 < lenth)											  // 如果整个区间尚未迭代完毕
		{
			half = lenth >> 1;										  // 找出中间位置

			middle = _RANGES next(first, half);						  // 设定中央迭代器

			if (_STD invoke(pred, _STD invoke(proj, *middle), value)) // 如果 中央元素 < 指定值
			{
				first = _STD move(middle);
				++first; // 将区间缩小（移至后半段），以提高效率

				lenth -= __cove_type((half + 1), difference_type);
			}
			else if (_STD invoke(pred, value, _STD invoke(proj, *middle))) // 如果 中央元素 > 指定值
			{
				lenth = half; // 将区间缩小（移至前半段），以提高效率
			}
			else			  // 如果 中央元素 == 指定值
			{
				// 在前半段寻找
				ForwardIterator begin { _STD move(lower_bound(first, middle, value, pred, proj)) };

				// 在后半段寻找
				ForwardIterator end { _STD move(
					upper_bound(_STD move(++middle), _STD move(first + lenth), _STD move(value), pred, proj)) };

				return { _STD move(begin), _STD move(end) };
			}
		}

		// 整个区间内没有匹配值，则返回一对迭代器--指向第一个 大于指定值 的元素
		return { first, first };
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function equal_range() for 仿函数 标准版 */
	template <_STD forward_iterator ForwardIterator,
			  _STD sentinel_for<ForwardIterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<ForwardIterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES subrange<ForwardIterator>
		operator()(ForwardIterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		auto	   check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto	   check_last  = __get_last_iterator_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));
		const auto lenth	   = check_last - check_first;

		auto result = __default_equal_range(_STD move(check_first),
											_STD move(lenth),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		return __rewrap_subrange<_RANGES subrange<ForwardIterator>>(first, _STD move(result));
	}

	/* function equal_range() for 容器、仿函数 强化版 */
	template <_RANGES forward_range Range,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(
			__basic_concept_for_binary_search_function<typename _RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		const auto lenth = _RANGES distance(rng);
		auto result		 = __default_equal_range(_RANGES begin(rng), _STD move(lenth), _STD move(value), pred, proj);
		auto first		 = _RANGES begin(rng);

		return __rewrap_subrange<_RANGES borrowed_iterator_t<Range>>(first, _STD move(result));
	}
};

constexpr inline __Equal_range_function equal_range { __Not_quite_object::__Construct_tag {} };

// 返回一个布尔值，表示有序序列中是否存在与给定值相同的元素。可指定谓词 pred 与投影 proj
struct __Binary_search_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <_STD forward_iterator ForwardIterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr bool __default_binary_search(ForwardIterator first,
															 ForwardIterator last,
															 const Type&	 value,
															 Predicate		 pred,
															 Projection		 proj) noexcept
	{
		ForwardIterator i { lower_bound(first, last, value, pred, proj) };

		return ((i != last) && !(_STD invoke(pred, value, _STD invoke(proj, *i))));
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function binary_search() for 仿函数 标准版 */
	template <_STD forward_iterator ForwardIterator,
			  _STD sentinel_for<ForwardIterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<ForwardIterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr bool operator()(ForwardIterator first,
																			Sentinel		last,
																			const Type&		value,
																			Predicate		pred = {},
																			Projection		proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));

		return __default_binary_search(_STD move(check_first),
									   _STD move(check_last),
									   _STD move(value),
									   __check_function(pred),
									   __check_function(proj));
	}

	/* function binary_search() for 容器、仿函数 强化版 */
	template <_RANGES forward_range Range,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(
			__basic_concept_for_binary_search_function<typename _RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(value), pred, proj);
	}
};

constexpr inline __Binary_search_function binary_search { __Not_quite_object::__Construct_tag {} };

__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
