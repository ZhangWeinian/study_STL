#pragma once

#include "00_basicFile.h"


#ifdef __HAS_CPP20

__BEGIN_NAMESPACE_ZHANG

struct __zh_No_inspection_required_function
{
	// 这是一个特别的标识符，如果某个函数具有未检查版本（即 不检查迭代器型别是否满足最低要求、
	// 不检查谓词是否满足要求、不检查投影是否满足要求、不检查是否满足自身的 requires （如果有））
	// 则在调用此函数的未检查版本时，只需将第一个参数设为此标识符的实例，之后再传入其他参数即可。
};

constexpr inline __zh_No_inspection_required_function __zh_Unchecked {};

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
_NODISCARD constexpr _RANGES iterator_t<Range> __rewrap_iterator(Range&& rng, Iterator&& iterator)
{
	if constexpr (_STD is_same_v<_STD remove_cvref_t<Iterator>, _RANGES iterator_t<Range>>)
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

/*------------------------------------------------------------------------------------------------*/



// 此处实现 copy()
struct __Copy_function: private __Not_quite_object
{
private:

	/* function __copy_with_random_access_iter() -- 辅助函数 */
	template <typename Iterator1, typename Iterator2>
	static constexpr Iterator2
		__copy_with_random_access_iter(Iterator1 first, Iterator1 last, Iterator2 result) noexcept
	{
		using difference_type = _STD iter_difference_t<Iterator1>;

		for (difference_type i { last - first }; i > 0; --i, ++result, ++first) // 以 i 决定循环的次数 -- 速度快
		{
			*result = *first;
		}

		return result;
	}

	// 完全泛化版本
	template <typename Iterator1, typename Sentinel, typename Iterator2>
	static constexpr Iterator2 __default_copy_dispatch(Iterator1 first, Sentinel last, Iterator2 result) noexcept
	{
		// 此处兵分两路：根据迭代器种类的不同，调用不同的 __copy_with_iter_tag() ，为的是不同种类的迭代器所使用的循环条件不同，有快慢之分
		if constexpr (_STD random_access_iterator<Iterator1>)
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
	template <typename Iterator1, typename Iterator2> // 完全泛化版本
	static constexpr Iterator2 __default_cpoy(Iterator1 first, Iterator1 last, Iterator2 result) noexcept
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
		auto check_last	 = __get_last_iterator_with_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

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
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr Type
		__default_accumulate(Iterator first, Iterator last, Type init, Predicate pred, Projection proj) noexcept
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
		auto check_last	 = __get_last_iterator_with_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

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
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr _STD iter_difference_t<Iterator>
		__default_count(Iterator first, Iterator last, const Type& value, Predicate pred, Projection proj) noexcept
	{
		using difference_type = _STD iter_difference_t<Iterator>;

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
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _STD iter_difference_t<InputIterator>
		operator()(InputIterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_with_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

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
												_STD projected<_RANGES iterator_t<Range>, Projection>,
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
	template <typename Iterator, typename Type>
	static constexpr void __default_itoa(Iterator first, Iterator last, Type value) noexcept
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
		auto check_last	 = __get_last_iterator_with_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));

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
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator
		__default_find(Iterator first, Iterator last, const Type& value, Predicate pred, Projection proj) noexcept
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
		auto check_last	 = __get_last_iterator_with_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

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
												_STD projected<_RANGES iterator_t<Range>, Projection>,
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
	template <typename Iterator1, typename Iterator2, typename Predicate, typename Projection1, typename Projection2>
	_NODISCARD static constexpr Iterator1 __default_find_first_of(Iterator1	  first1,
																  Iterator1	  last1,
																  Iterator2	  first2,
																  Iterator2	  last2,
																  Predicate	  pred,
																  Projection1 proj1,
																  Projection2 proj2) noexcept
	{
		for (; first1 != last1; ++first1)
		{
			for (Iterator2 i = first2; i != last2; ++i)
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
			__get_last_iterator_with_unwrapped<InputIterator, Sentinel_InputIterator>(check_first1, _STD move(last1));
		auto check_last2 =
			__get_last_iterator_with_unwrapped<ForwardIterator, Sentinel_ForwardIterator>(check_first2,
																						  _STD move(last2));

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
			  _RANGES forward_range		   Range,
			  typename Predicate   = _RANGES equal_to,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD indirectly_comparable<_RANGES iterator_t<InputRange>,
											_RANGES iterator_t<Range>,
											Predicate,
											Projection1,
											Projection2>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto operator()(InputRange&& rng1,
																			Range&&		 rng2,
																			Predicate	 pred  = {},
																			Projection1	 proj1 = {},
																			Projection2	 proj2 = {}) const noexcept
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
		Type tmp(static_cast<Type&&>(a));
		a = static_cast<Type&&>(b);
		b = static_cast<Type&&>(tmp);
	}
};

constexpr inline __Swap_function swap { __Not_quite_object::__Construct_tag {} };

// 交换 a 与 b 的值。不可指定谓词 pred 与投影 proj
struct __Iter_swap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	template <typename Iterator1, typename Iterator2>
	constexpr void operator()(Iterator1 a, Iterator2 b) const
		noexcept(noexcept(swap(*(static_cast<Iterator1&&>(a)), *(static_cast<Iterator2&&>(b)))))
	{
		swap(*(static_cast<Iterator1&&>(a)), *(static_cast<Iterator2&&>(b)));
	}
};

constexpr inline __Iter_swap_function iter_swap { __Not_quite_object::__Construct_tag {} };

// 将 [first1, last1) 范围内的元素与 [first2, first2 + (last1 - first1)) 范围内的元素互相交换。不可指定谓词 pred 与投影 proj
struct __Swap_ranges_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator1, typename Iterator2>
	static constexpr Iterator2 __default_swap_ranges(Iterator1 first1, Iterator1 last1, Iterator2 first2) noexcept
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
			__get_last_iterator_with_unwrapped<ForwardIterator1, Sentinel_ForwardIterator1>(check_first1,
																							_STD move(last1));

		auto check_first2 = _STD move(first2);

		return __default_swap_ranges(_STD move(check_first1), _STD move(check_last1), _STD move(check_first2));
	}

	/* function swap_ranges() for 容器 强化版 */
	template <_RANGES forward_range Range>
		requires(_STD indirectly_swappable<_RANGES iterator_t<Range>, _RANGES iterator_t<Range>>)
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
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr Predicate
		__default_for_each(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
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
		auto check_last	 = __get_last_iterator_with_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		return __default_for_each(_STD move(check_first),
								  _STD move(check_last),
								  __check_function(pred),
								  __check_function(proj));
	}

	/* function for_each() for 容器 强化版 */
	template <_RANGES input_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirectly_unary_invocable<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate>
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
	template <typename Iterator1, typename Iterator2, typename Predicate, typename Projection1, typename Projection2>
	_NODISCARD static constexpr bool __default_equal(Iterator1	 first1,
													 Iterator1	 last1,
													 Iterator2	 first2,
													 Iterator2	 last2,
													 Predicate	 pred,
													 Projection1 proj1,
													 Projection2 proj2) noexcept
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
	template <typename Iterator, typename Type>
	static constexpr void __default_fill(Iterator first, Iterator last, const Type& value) noexcept
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
	template <typename Iterator, typename Type>
	static constexpr Iterator
		__default_fill_n(Iterator first, _STD iter_difference_t<Iterator> n, const Type& value) noexcept
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
	constexpr OutputIterator
		operator()(OutputIterator first, _STD iter_difference_t<OutputIterator> n, const Type& value) const noexcept
	{
		return __default_fill_n(_STD move(first), _STD move(n), _STD move(value));
	};

	/* function fill_n() for 容器 强化版 */
	template <typename Type, _RANGES output_range<const Type&> Range>
	constexpr auto operator()(Range&& rng, _RANGES range_difference_t<Range> n, const Type& value) const noexcept
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
	template <
		_RANGES input_range		   Range,
		typename Projection = _STD identity,
		_STD indirect_strict_weak_order<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate = _RANGES less>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				*(ubegin(rng) + 1)
			} noexcept;

			{
				_STD invoke(pred, _STD invoke(proj, *(ubegin(rng))), _STD invoke(proj, *(ubegin(rng) + 1)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const _RANGES range_value_t<Range>&
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		auto first = ubegin(rng);
		auto last  = uend(rng);
		auto ans   = first;

		for (; first != last; ++first)
		{
			ans =
				(_STD invoke(check_pred, _STD invoke(check_proj, *first), _STD invoke(check_proj, *ans))) ? ans : first;
		}

		return *ans;
	}

	/* function max() for initializer_list 强化版 */
	template <_STD copyable				 Type,
			  typename Projection																 = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<const Type*, Projection>> Predicate = _RANGES less>
		requires(requires(_STD initializer_list<Type> rng, Predicate pred, Projection proj) {
			{
				*(ubegin(rng) + 1)
			} noexcept;

			{
				_STD invoke(pred, _STD invoke(proj, *(ubegin(rng))), _STD invoke(proj, *(ubegin(rng) + 1)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const Type&
		operator()(_STD initializer_list<Type> rng, Predicate pred = {}, Projection proj = {}) const
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		auto first = rng.begin();
		auto last  = rng.end();
		auto ans   = first;

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
			  _STD indirect_strict_weak_order<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate =
				  _RANGES																			 greater>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				*(ubegin(rng) + 1)
			} noexcept;

			{
				_STD invoke(pred, _STD invoke(proj, *(ubegin(rng))), _STD invoke(proj, *(ubegin(rng) + 1)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const _RANGES range_value_t<Range>&
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		auto first = ubegin(rng);
		auto last  = uend(rng);
		auto ans   = first;

		for (; first != last; ++first)
		{
			ans =
				(_STD invoke(check_pred, _STD invoke(check_proj, *first), _STD invoke(check_proj, *ans))) ? ans : first;
		}

		return *ans;
	}

	/* function min() for initializer_list 强化版 */
	template <_STD copyable				 Type,
			  typename Projection																 = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<const Type*, Projection>> Predicate = _RANGES greater>
		requires(requires(_STD initializer_list<Type> rng, Predicate pred, Projection proj) {
			{
				*(ubegin(rng) + 1)
			} noexcept;

			{
				_STD invoke(pred, _STD invoke(proj, *(ubegin(rng))), _STD invoke(proj, *(ubegin(rng) + 1)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const Type&
		operator()(_STD initializer_list<Type> rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_pred = __check_function(pred);
		auto check_proj = __check_function(proj);

		auto first = rng.begin();
		auto last  = rng.end();
		auto ans   = first;

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
	template <typename Iterator, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator
		__default_max_element(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
	{
		if (first == last)
		{
			return first;
		}

		Iterator result { first };
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
		return __seek_wrapped(first,
							  __default_max_element(__unwrap_iterator<Sentinel>(_STD move(first)),
													__unwrap_sentinel<ForwardIterator, Sentinel>(_STD move(last)),
													__check_function(pred),
													__check_function(proj)));
	}

	/* function max_element() for 容器、仿函数 强化版 */
	template <
		_RANGES forward_range	   Range,
		typename Projection = _STD identity,
		_STD indirect_strict_weak_order<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate = _RANGES less>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, *(ubegin(rng))), _STD invoke(proj, *(ubegin(rng))))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return __rewrap_iterator(
			rng,
			__default_max_element(ubegin(rng), uend(rng), __check_function(pred), __check_function(proj)));
	}
};

constexpr inline __Max_element_function max_element { __Not_quite_object::__Construct_tag {} };

// 返回 [first, last) 范围内的最小元素。可指定谓词 pred 与投影 proj
struct __Min_element_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator
		__default_min_element(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
	{
		if (first == last)
		{
			return first;
		}

		Iterator result { first };
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
		return __seek_wrapped(first,
							  __default_min_element(__unwrap_iterator<Sentinel>(_STD move(first)),
													__unwrap_sentinel<ForwardIterator, Sentinel>(_STD move(last)),
													__check_function(pred),
													__check_function(proj)));
	}

	/* function min_element() for 容器、仿函数 强化版 */
	template <_RANGES forward_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate =
				  _RANGES																			 greater>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			{
				_STD invoke(pred, _STD invoke(proj, *(ubegin(rng))), _STD invoke(proj, *(ubegin(rng))))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return __rewrap_iterator(
			rng,
			__default_min_element(ubegin(rng), uend(rng), __check_function(pred), __check_function(proj)));
	}
};

constexpr inline __Min_element_function min_element { __Not_quite_object::__Construct_tag {} };

// 将 [first1, last1) 与 [first2, last2) 保持有序合并到 result 中。可指定谓词 pred 与投影 proj1、proj2
struct __Merge_function: private __Not_quite_object
{
private:

	template <typename InputIterator1, typename InputIterator2, typename OutputIterator>
	using merge_result = _RANGES in_in_out_result<InputIterator1, InputIterator2, OutputIterator>;

	// 统一调用方式
	template <typename Iterator1,
			  typename Iterator2,
			  typename Iterator3,
			  typename Predicate,
			  typename Projection1,
			  typename Projection2>
	_NODISCARD static constexpr merge_result<Iterator1, Iterator2, Iterator3>
		__default_merge(Iterator1	first1,
						Iterator1	last1,
						Iterator2	first2,
						Iterator2	last2,
						Iterator3	result,
						Predicate	pred,
						Projection1 proj1,
						Projection2 proj2) noexcept
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

		// 最后剩余元素拷贝到目的端。（两个序列一定至少有一个为空）
		if (first1 == last1)
		{
			auto [end_for_range1, end_for_range_2] =
				_RANGES copy(_STD move(first2), _STD move(last2), _STD move(result));
			return { _STD move(first1), _STD move(end_for_range1), _STD move(end_for_range_2) };
		}
		else // first2 == last2
		{
			auto [end_for_range1, end_for_range_2] =
				_RANGES copy(_STD move(first1), _STD move(last1), _STD move(result));
			return { _STD move(end_for_range1), _STD move(first2), _STD move(end_for_range_2) };
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function merge() for 仿函数 标准版 */
	template <_STD input_iterator InputIterator1,
			  _STD input_iterator InputIterator2,
			  _STD sentinel_for<InputIterator1> Sentinel_InputIterator1,
			  _STD sentinel_for<InputIterator2> Sentinel_InputIterator2,
			  _STD weakly_incrementable			OutputIterator,
			  typename Predicate   = _RANGES	  less,
			  typename Projection1 = _STD		identity,
			  typename Projection2 = _STD		identity>
		requires(_STD mergeable<InputIterator1, InputIterator2, OutputIterator, Predicate, Projection1, Projection2>)
	constexpr merge_result<InputIterator1, InputIterator2, OutputIterator>
		operator()(InputIterator1		   first1,
				   Sentinel_InputIterator1 last1,
				   InputIterator2		   first2,
				   Sentinel_InputIterator2 last2,
				   OutputIterator		   result,
				   Predicate			   pred	 = {},
				   Projection1			   proj1 = {},
				   Projection2			   proj2 = {}) const noexcept
	{
		auto [end_for_range1, end_for_range2, end_for_range3] =
			__default_merge(__unwrap_iterator<Sentinel_InputIterator1>(_STD move(first1)),
							__unwrap_sentinel<InputIterator1, Sentinel_InputIterator1>(_STD move(last1)),
							__unwrap_iterator<Sentinel_InputIterator2>(_STD move(first2)),
							__unwrap_sentinel<InputIterator2, Sentinel_InputIterator2>(_STD move(last2)),
							_STD move(result),
							__check_function(pred),
							__check_function(proj1),
							__check_function(proj2));

		__seek_wrapped(first1, _STD move(end_for_range1));
		__seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2), _STD move(end_for_range3) };
	}

	/* function merge() for 容器、仿函数 强化版 */
	template <_RANGES input_range		   InputRange1,
			  _RANGES input_range		   InputRange2,
			  _STD weakly_incrementable	   OutputIterator,
			  typename Predicate   = _RANGES less,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD mergeable<_RANGES iterator_t<InputRange1>,
								_RANGES iterator_t<InputRange2>,
								OutputIterator,
								Predicate,
								Projection1,
								Projection2>)
	constexpr merge_result<_RANGES borrowed_iterator_t<InputRange1>,
						   _RANGES borrowed_iterator_t<InputRange2>,
						   OutputIterator>
		operator()(InputRange1&&  rng1,
				   InputRange2&&  rng2,
				   OutputIterator result,
				   Predicate	  pred	= {},
				   Projection1	  proj1 = {},
				   Projection2	  proj2 = {}) const noexcept
	{
		auto first1 = _RANGES begin(rng1);
		auto first2 = _RANGES begin(rng2);

		auto [end_for_range1, end_for_range2, end_for_range3] =
			__default_merge(__unwrap_range_iterator<InputRange1>(_STD move(first1)),
							uend(rng1),
							__unwrap_range_iterator<InputRange2>(_STD move(first2)),
							uend(rng2),
							_STD move(result),
							__check_function(pred),
							__check_function(proj1),
							__check_function(proj2));

		__seek_wrapped(first1, _STD move(end_for_range1));
		__seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2), _STD move(end_for_range3) };
	}
};

constexpr inline __Merge_function merge { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素依次调用 pred ，并将结果保存到 result 中。可指定谓词 pred 与投影 proj
struct __Transform_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator1, typename Iterator2, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator2
		__default_transform(Iterator1 first, Iterator1 last, Iterator2 result, Predicate pred, Projection proj) noexcept
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
		auto check_last	 = __get_last_iterator_with_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

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
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void
		__default_partial_sort(Iterator first, Iterator middle, Iterator last, Predicate pred, Projection proj) noexcept
	{
		using value_type = _STD iter_value_t<Iterator>;

		_RANGES make_heap(first, middle, pred, proj);

		for (Iterator i { middle }; i != last; ++i)
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
		auto check_last =
			__get_last_iterator_with_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__default_partial_sort(_STD move(check_first),
							   _STD move(check_middle),
							   _STD move(check_last),
							   __check_function(pred),
							   __check_function(proj));
	}

	/* function partial_sort() for 容器、仿函数 强化版 */
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr auto operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pred, proj);
	}

	// partial_sort() 的非检查版本
	template <typename Iterator, typename Sentinel, typename Predicate, typename Projection>
	constexpr void operator()(__zh_No_inspection_required_function,
							  Iterator	 first,
							  Iterator	 middle,
							  Sentinel	 last,
							  Predicate	 pred,
							  Projection proj) const noexcept
	{
		__default_partial_sort(_STD move(first), _STD move(middle), _STD move(last), pred, proj);
	}
};

constexpr inline __Partial_sort_function partial_sort { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素插入排序。可指定谓词 pred 与投影 proj
struct __Insertion_sort_function: private __Not_quite_object
{
private:

	// 插入排序 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr Iterator
		__default_insertion_sort(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
	{
		if (first == last)
		{
			return first;
		}

		using value_type = _STD iter_value_t<Iterator>;

		for (Iterator i { first }; ++i != last;)
		{
			value_type value { _RANGES iter_move(i) };

			Iterator tmp { i };

			for (Iterator j { tmp }; tmp != first;)
			{
				--j;

				if (!(_STD invoke(pred, _STD invoke(proj, value), _STD invoke(proj, *j))))
				{
					break;
				}

				*tmp = _RANGES iter_move(j);

				--tmp;
			}

			*tmp = _STD move(value);
		}

		return last;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// insertion_sort() for 仿函数 标准版
	template <_STD bidirectional_iterator BidirectionalIterator,
			  _STD sentinel_for<BidirectionalIterator> Sentinel,
			  typename Predicate  = _RANGES				less,
			  typename Projection = _STD			   identity>
		requires(__basic_concept_for_sort_function<BidirectionalIterator, Predicate, Projection>)
	constexpr BidirectionalIterator
		operator()(BidirectionalIterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(first);
		auto check_last =
			__get_last_iterator_with_unwrapped<BidirectionalIterator, Sentinel>(check_first, _STD move(last));

		check_first = __default_insertion_sort(_STD move(check_first),
											   _STD move(check_last),
											   __check_function(pred),
											   __check_function(proj));

		__seek_wrapped(first, check_first);

		return first;
	}

	// insertion_sort()	for 容器、仿函数 强化版
	template <_RANGES bidirectional_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto result = __default_insertion_sort(_RANGES begin(rng), _RANGES end(rng), pred, proj);

		return __rewrap_iterator(rng, _STD move(result));
	}

	// insertion_sort() 的非检查版本
	template <typename Iterator, typename Sentinel, typename Predicate, typename Projection>
	constexpr Iterator operator()(__zh_No_inspection_required_function,
								  Iterator	 first,
								  Sentinel	 last,
								  Predicate	 pred,
								  Projection proj) const noexcept
	{
		return __default_insertion_sort(_STD move(first), _STD move(last), pred, proj);
	}
};

constexpr inline __Insertion_sort_function insertion_sort { __Not_quite_object::__Construct_tag {} };

// 此处实现 __zh_Set_median_of_three_with_unchecked() 设置 “三点中值” 。原本服务于 quick_sort() ，单独写出是为了在其他地方调用
struct __zh_Set_median_of_three_with_uncheckde_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void __default_set_median_of_three(Iterator   first,
														Iterator   mid,
														Iterator   last,
														Predicate  pred,
														Projection proj) noexcept
	{
		if (_STD invoke(pred, _STD invoke(proj, *mid), _STD invoke(proj, *first)))
		{
			_RANGES iter_swap(mid, first);
		}

		if (!(_STD invoke(pred, _STD invoke(proj, *last), _STD invoke(proj, *mid))))
		{
			return;
		}

		_RANGES iter_swap(last, mid);

		if (_STD invoke(pred, _STD invoke(proj, *mid), _STD invoke(proj, *first)))
		{
			_RANGES iter_swap(mid, first);
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	template <_STD random_access_iterator RandomAccessIterator, typename Predicate, typename Projection>
	constexpr void operator()(RandomAccessIterator first,
							  RandomAccessIterator mid,
							  RandomAccessIterator last,
							  Predicate			   pred,
							  Projection		   proj) const noexcept
	{
		using difference_type			   = _STD iter_difference_t<RandomAccessIterator>;
		const difference_type		 lenth = last - first;

		if ((__max_get_median_of_three_constant<difference_type>) < lenth)
		{
			const difference_type step1 = (lenth + 1) >> 3; // +1 不会溢出，因为在调用方中使范围包含在内
			const difference_type step2 = step1 << 1;		// 注意：有意丢弃低位

			__default_set_median_of_three(first, first + step1, first + step2, pred, proj);
			__default_set_median_of_three(mid - step1, mid, mid + step1, pred, proj);
			__default_set_median_of_three(last - step2, last - step1, last, pred, proj);
			__default_set_median_of_three(first + step1, mid, last - step1, pred, proj);
		}
		else
		{
			__default_set_median_of_three(first, mid, last, pred, proj);
		}
	}
};

constexpr inline __zh_Set_median_of_three_with_uncheckde_function __zh_Set_median_of_three_with_unchecked {
	__Not_quite_object::__Construct_tag {}
};

// 此处实现 __zh_Partition_with_unchecked() 进行无边界检查的 分割序列 。原本服务于 quick_sort() ，单独写出是为了在其他地方调用
struct __zh_Partition_with_unchecked_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr _RANGES subrange<Iterator>
		__default_unguraded_partition(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
	{
		Iterator mid = first + ((last - first) >> 1);

		__zh_Set_median_of_three_with_unchecked(first, mid, _RANGES prev(last), pred, proj);

		Iterator pfirst = mid;
		Iterator plast	= _RANGES next(pfirst);

		while ((first < pfirst) &&
			   (!(_STD invoke(pred, _STD invoke(proj, *(_RANGES prev(pfirst))), _STD invoke(proj, *pfirst)))) &&
			   (!(_STD invoke(pred, _STD invoke(proj, *pfirst), _STD invoke(proj, *(_RANGES prev(pfirst)))))))
		{
			--pfirst;
		}

		while ((plast < last) && (!(_STD invoke(pred, _STD invoke(proj, *plast), _STD invoke(proj, *pfirst)))) &&
			   (!(_STD invoke(pred, _STD invoke(proj, *pfirst), _STD invoke(proj, *plast)))))
		{
			++plast;
		}

		Iterator gfirst = plast;
		Iterator glast	= pfirst;

		while (true)
		{
			for (; gfirst < last; ++gfirst)
			{
				if (_STD invoke(pred, _STD invoke(proj, *pfirst), _STD invoke(proj, *gfirst)))
				{
					continue;
				}
				else if (_STD invoke(pred, _STD invoke(proj, *gfirst), _STD invoke(proj, *pfirst)))
				{
					break;
				}
				else if (plast != gfirst)
				{
					_RANGES iter_swap(plast, gfirst);

					++plast;
				}
				else
				{
					++plast;
				}
			}

			for (; first < glast; --glast)
			{
				if (_STD invoke(pred, _STD invoke(proj, *(_RANGES prev(glast))), _STD invoke(proj, *pfirst)))
				{
					continue;
				}
				else if (_STD invoke(pred, _STD invoke(proj, *pfirst), _STD invoke(proj, *(_RANGES prev(glast)))))
				{
					break;
				}
				else if (--pfirst != _RANGES prev(glast))
				{
					_RANGES iter_swap(pfirst, _RANGES prev(glast));
				}
			}

			if (glast == first && gfirst == last)
			{
				return { _STD move(pfirst), _STD move(plast) };
			}

			if (glast == first)
			{
				// 底部没有空间，向上旋转枢轴
				if (plast != gfirst)
				{
					_RANGES iter_swap(pfirst, plast);
				}

				++plast;

				_RANGES iter_swap(pfirst, gfirst);

				++pfirst;
				++gfirst;
			}
			else if (gfirst == last)
			{
				// 顶部没有空间，向下旋转枢轴
				if (--glast != --pfirst)
				{
					_RANGES iter_swap(glast, pfirst);
				}

				_RANGES iter_swap(pfirst, --plast);
			}
			else
			{
				_RANGES iter_swap(gfirst, --glast);

				++gfirst;
			}
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// 用于分割序列
	template <_STD random_access_iterator RandomAccessIterator,
			  _STD sentinel_for<RandomAccessIterator> Sentinel,
			  typename Predicate,
			  typename Projection>
	constexpr _RANGES subrange<RandomAccessIterator>
		operator()(RandomAccessIterator first, Sentinel last, Predicate pred, Projection proj) const noexcept
	{
		// 选择一个枢轴，将[first，last）划分为小于枢轴的元素、等于枢轴的元素和大于枢轴的元素；将相等的分区作为子范围返回。

		return __default_unguraded_partition(_STD move(first), _STD move(last), pred, proj);
	}
};

constexpr inline __zh_Partition_with_unchecked_function __zh_Partition_with_unchecked {
	__Not_quite_object::__Construct_tag {}
};

// 将 [first, last) 范围内的元素快速排序。可指定谓词 pred 与投影 proj
struct __Quick_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void __default_quick_sort(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
	{
		using value_type = _STD iter_value_t<Iterator>;

		while (1 < (last - first))
		{
			auto [pfirst, plast] = __zh_Partition_with_unchecked(first, last, pred, proj);

			if ((last - plast) < (pfirst - first))
			{
				__default_quick_sort(plast, last, pred, proj);
				last = _STD move(plast);
			}
			else
			{
				__default_quick_sort(first, pfirst, pred, proj);
				first = _STD move(pfirst);
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
		auto check_last =
			__get_last_iterator_with_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__default_quick_sort(_STD move(check_first),
							 _STD move(check_last),
							 __check_function(pred),
							 __check_function(proj));

		return ans;
	}

	// quick_sort() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto result = __default_quick_sort(_RANGES begin(rng), _RANGES end(rng), pred, proj);

		return __rewrap_iterator(rng, _STD move(result));
	}
};

constexpr inline __Quick_sort_function quick_sort { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素归并排序。可指定谓词 pred 与投影 proj
struct __Merge_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void __default_merge_sort(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
	{
		using difference_type = _STD iter_difference_t<Iterator>;
		difference_type n	  = _RANGES distance(first, last);

		if ((n == 0) || (n == 1))
		{
			return;
		}

		Iterator mid { first + (n / 2) };

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
		auto check_last =
			__get_last_iterator_with_unwrapped<BidirectionalIterator, Sentinel>(check_first, _STD move(last));

		__default_merge_sort(_STD move(check_first),
							 _STD move(check_last),
							 __check_function(pred),
							 __check_function(proj));

		return ans;
	}

	// merge_sort() for 容器、仿函数 强化版
	template <_RANGES bidirectional_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
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

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void __default_sort(Iterator first,
										 Iterator last,
										 _STD iter_difference_t<Iterator> ideal,
										 Predicate						  pred,
										 Projection						  proj)
	{
		while (true)
		{
			if ((last - first) <
				(__stl_threshold<_STD iter_difference_t<Iterator>>)) // 使用插入排序，如果元素数量足够少
			{
				insertion_sort(__zh_Unchecked, _STD move(first), _STD move(last), pred, proj);

				return;
			}

			if (ideal == 0) // 使用堆排序，如果递归深度足够深
			{
				partial_sort(__zh_Unchecked, _STD move(first), last, _STD move(last), pred, proj);

				return;
			}

			ideal = (ideal >> 1) + (ideal >> 2); // allow 1.5 log2(N) divisions

			// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
			auto [pfirst, plast] = __zh_Partition_with_unchecked(first, last, pred, proj);

			// 递归调用，优先对较短的序列进行排序
			if (pfirst - first < last - plast)
			{
				__default_sort(first, _STD move(pfirst), ideal, pred, proj);

				first = _STD move(plast);
			}
			else
			{
				__default_sort(_STD move(plast), last, ideal, pred, proj);

				last = _STD move(pfirst);
			}
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
		auto check_first = __unwrap_iterator<Sentinel>(first);
		auto check_last =
			__get_last_iterator_with_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

		__seek_wrapped(first, check_first);

		const auto lenth = check_last - check_first;

		__default_sort(_STD move(check_first),
					   _STD move(check_last),
					   _STD move(lenth),
					   __check_function(pred),
					   __check_function(proj));

		return first;
	}

	// sort() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto	   check_first = ubegin(rng);
		auto	   check_last  = __get_last_iterator_with_unwrapped(rng);
		const auto lenth	   = check_last - check_first;

		__default_sort(_STD move(check_first), check_last, lenth, pred, proj);

		return __rewrap_iterator(rng, _STD move(check_first));
	}
};

constexpr inline __Sort_function sort { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素进行分割，使得 [first, nth) 范围内的元素均小于等于 [nth, last) 范围内的元素。可指定谓词 pred 与投影 proj
struct __Nth_element_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void
		__nth_element(Iterator first, Iterator nth, Iterator last, Predicate pred, Projection proj) noexcept
	{
		while (3 < (last - first))
		{
			auto [pfirst, plast] = __zh_Partition_with_unchecked(first, last, pred, proj);

			if (nth < pfirst)			   // 如果 指定位置 < 右段起点，（即 nth 位于右段）
			{
				first = _STD move(pfirst); // 对右段实施分割
			}
			else						   // 否则（nth 位于左段）
			{
				last = _STD move(plast);   // 对左段实施分割
			}
		}

		insertion_sort(__zh_Unchecked, _STD move(first), _STD move(last), pred, proj);
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
		auto check_last =
			__get_last_iterator_with_unwrapped<RandomAccessIterator, Sentinel>(check_first, _STD move(last));

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

// 返回一个有序序列中第一个小于等于给定值的位置。可指定谓词 pred 与投影 proj
struct __Lower_bound_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator __default_lower_bound(Iterator first,
															   _STD iter_difference_t<Iterator> lenth,
															   const Type&						value,
															   Predicate						pred,
															   Projection						proj) noexcept
	{
		using difference_type = _STD iter_difference_t<Iterator>;

		difference_type half { 0 };

		Iterator middle { first };

		while (0 < lenth)
		{
			half = lenth >> 1;

			middle = _RANGES next(first, half);

			if (_STD invoke(pred, _STD invoke(proj, *middle), value))
			{
				first = _STD move(middle);
				++first;

				lenth -= static_cast<difference_type>(half + 1);
			}
			else
			{
				lenth = _STD move(half);
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
	template <_RANGES forward_range Range,
			  typename Type,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<const Type*, _STD projected<_RANGES iterator_t<Range>, Projection>>
				  Predicate = _RANGES less>
		requires(__basic_concept_for_binary_search_function<_RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto result = __default_lower_bound(ubegin(rng),
											_STD move(_RANGES distance(rng)),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		return __rewrap_iterator(rng, _STD move(result));
	}

	// lower_bound() 的非检查版本
	template <typename Iterator, typename Sentinel, typename Type, typename Predicate, typename Projection>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr Iterator operator()(__zh_No_inspection_required_function,
								  Iterator	  first,
								  Sentinel	  last,
								  const Type& value,
								  Predicate	  pred,
								  Projection  proj) const noexcept
	{
		return __default_lower_bound(_STD move(first), _STD move(last - first), _STD move(value), pred, proj);
	}
};

constexpr inline __Lower_bound_function lower_bound { __Not_quite_object::__Construct_tag {} };

// 返回一个有序序列中第一个大于等于给定值的位置。可指定谓词 pred 与投影 proj
struct __Upper_bound_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator __default_upper_bound(Iterator first,
															   _STD iter_difference_t<Iterator> lenth,
															   const Type&						value,
															   Predicate						pred,
															   Projection						proj) noexcept
	{
		using difference_type = _STD iter_difference_t<Iterator>;

		difference_type half { 0 };

		Iterator middle { first };

		while (0 < lenth)
		{
			half = lenth >> 1;

			middle = _RANGES next(first, half);

			if (_STD invoke(pred, value, _STD invoke(proj, *middle)))
			{
				lenth = _STD move(half);
			}
			else
			{
				first = _STD move(middle);
				++first;

				lenth -= static_cast<difference_type>(half + 1);
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
		requires(__basic_concept_for_binary_search_function<_RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto result = __default_upper_bound(ubegin(rng),
											_STD move(_RANGES distance(rng)),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		return __rewrap_iterator(rng, _STD move(result));
	}

	// upper_bound() 的非检查版本
	template <typename Iterator, typename Sentinel, typename Type, typename Predicate, typename Projection>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr Iterator operator()(__zh_No_inspection_required_function,
								  Iterator	  first,
								  Sentinel	  last,
								  const Type& value,
								  Predicate	  pred,
								  Projection  proj) const noexcept
	{
		return __default_upper_bound(_STD move(first), _STD move(last - first), _STD move(value), pred, proj);
	}
};

constexpr inline __Upper_bound_function upper_bound { __Not_quite_object::__Construct_tag {} };

// 返回一个有序序列中与给定值相同的 begin 位置和 end 位置。可指定谓词 pred 与投影 proj
struct __Equal_range_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr _RANGES subrange<Iterator> __default_equal_range(Iterator first,
																				 _STD iter_difference_t<Iterator> lenth,
																				 const Type&					  value,
																				 Predicate						  pred,
																				 Projection proj) noexcept
	{
		using difference_type = _STD iter_difference_t<Iterator>;

		difference_type half { 0 };

		Iterator middle { first };

		while (0 < lenth)											  // 如果整个区间尚未迭代完毕
		{
			half = lenth >> 1;										  // 找出中间位置

			middle = _RANGES next(first, half);						  // 设定中央迭代器

			if (_STD invoke(pred, _STD invoke(proj, *middle), value)) // 如果 中央元素 < 指定值
			{
				first = _STD move(middle);
				++first; // 将区间缩小（移至后半段），以提高效率

				lenth -= static_cast<difference_type>(half + 1);
			}
			else if (_STD invoke(pred, value, _STD invoke(proj, *middle))) // 如果 中央元素 > 指定值
			{
				lenth = _STD move(half); // 将区间缩小（移至前半段），以提高效率
			}
			else						 // 如果 中央元素 == 指定值
			{
				// 在前半段寻找
				auto begin { _STD move(lower_bound(__zh_Unchecked, first, middle, value, pred, proj)) };

				// 在后半段寻找
				auto end { _STD move(upper_bound(__zh_Unchecked,
												 _STD move(++middle),
												 _STD move(first + lenth),
												 _STD move(value),
												 pred,
												 proj)) };

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
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_with_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));
		const auto lenth = check_last - check_first;

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
		requires(__basic_concept_for_binary_search_function<_RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto result = __default_equal_range(_STD move(ubegin(rng)),
											_STD move(_RANGES distance(rng)),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		return __rewrap_subrange<_RANGES borrowed_iterator_t<Range>>(first, _STD move(result));
	}
};

constexpr inline __Equal_range_function equal_range { __Not_quite_object::__Construct_tag {} };

// 返回一个布尔值，表示有序序列中是否存在与给定值相同的元素。可指定谓词 pred 与投影 proj
struct __Binary_search_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr bool __default_binary_search(Iterator	 first,
															 Iterator	 last,
															 const Type& value,
															 Predicate	 pred,
															 Projection	 proj) noexcept
	{
		Iterator i { lower_bound(__zh_Unchecked, first, last, value, pred, proj) };

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
		auto check_last	 = __get_last_iterator_with_unwrapped<ForwardIterator, Sentinel>(check_first, _STD move(last));

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
		requires(__basic_concept_for_binary_search_function<_RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return (*this)(_RANGES begin(rng), _RANGES end(rng), _STD move(value), pred, proj);
	}
};

constexpr inline __Binary_search_function binary_search { __Not_quite_object::__Construct_tag {} };

__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
