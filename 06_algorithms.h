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

constexpr inline __zh_No_inspection_required_function __zh_No_inspection_required {};

// 此函数的作用是将一个迭代器解包，返回一个裸指针
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

// 此函数的作用是将一个迭代器解包，返回一个裸指针
template <_RANGES forward_range Range, class Iterator>
_NODISCARD constexpr _RANGES iterator_t<Range> __rewrap_iterator(Range&& rng, Iterator&& iter)
{
	if constexpr (_STD is_same_v<_STD remove_cvref_t<Iterator>, _RANGES iterator_t<Range>>)
	{
		return _STD forward<Iterator>(iter);
	}
	else
	{
		auto result = _RANGES begin(rng);

		result._Seek_to(_STD forward<Iterator>(iter));

		return result;
	}
}

template <typename Iterator, typename UIterator>
concept __wrapped_seekable =
	requires(Iterator iter, UIterator uiter) { iter._Seek_to(_STD forward<UIterator>(uiter)); };

// 此函数的作用是将 uiter（通常是一个裸指针） 的信息，装载到 iter（通常是一个迭代器）
template <typename Iterator, typename UIterator>
constexpr void __seek_wrapped(Iterator& iter, UIterator&& uiter)
{
	if constexpr (__wrapped_seekable<Iterator, UIterator>)
	{
		iter._Seek_to(_STD forward<UIterator>(uiter));
	}
	else
	{
		iter = _STD forward<UIterator>(uiter);
	}
}

/*------------------------------------------------------------------------------------------------*/

// 此处实现 copy()
struct __Copy_function: private __Not_quite_object
{
private:

	template <class Iterator, class OutIter>
	using copy_result = _RANGES in_out_result<Iterator, OutIter>;

	// 统一调用方式
	template <typename Iterator, typename Sentinel, typename OutIter>
	static constexpr OutIter __default_cpoy(Iterator first, Sentinel last, OutIter result) noexcept
	{
		using value_t = _STD iter_value_t<Iterator>;
		using diff_t  = _STD  iter_difference_t<Iterator>;

		if constexpr ((_STD is_same_v<_STD remove_cvref_t<Iterator>, _STD remove_cvref_t<OutIter>>)&&(
						  (_STD is_same_v<_STD remove_cv_t<Iterator>,
										  _STD conditional_t<_STD is_const_v<Iterator>, const char*, char*>>)
						  || (_STD is_same_v<
							  _STD remove_cv_t<Iterator>,
							  _STD conditional_t<
								  _STD is_const_v<Iterator>,
								  const wchar_t*,
								  wchar_t*>>)))	 // 如果是 char* 或 wchar_t* 或它们的 const 形式，则调用 memmove()
		{
			_STD invoke(_CSTD memmove, result, first, sizeof(value_t) * (last - first));

			return result + (last - first);
		}
		else if constexpr ((_STD is_pointer_v<Iterator>)&&(_STD is_same_v<Iterator, Sentinel>)&&(
							   _STD is_same_v<Iterator, OutIter>))	// 如果 first 、last 与 result 是同类型的指针
		{
			if constexpr (_STD is_trivially_assignable_v<
							  _STD iter_value_t<OutIter>,
							  _STD iter_value_t<
								  Iterator>>)  // 以下版本适用于 “指针所指之对象，具备 trivial assignment operator ”
			{
				__invoke(_CSTD memmove, result, first, sizeof(value_t) * (last - first));

				return result + (last - first);
			}

			// 以下版本适用于 “指针所指之对象，具备 non-trivial assignment operator ”
			for (diff_t i { last - first }; i > 0; --i, ++result, ++first)	// 以 i 决定循环的次数 -- 速度快
			{
				*result = *first;
			}

			return result;
		}
		else if constexpr (_STD random_access_iterator<Iterator>)  // 如果是随机访问迭代器
		{
			for (diff_t i { last - first }; i > 0; --i, ++result, ++first)	// 以 i 决定循环的次数 -- 速度快
			{
				*result = *first;
			}

			return result;
		}
		else  // 这是兜底手段
		{
			for (; first != last; ++result, ++first)  // 以迭代器相同与否，决定循环是否继续 -- 速度慢
			{
				*result = *first;
			}

			return result;
		}
	}


public:

	using __Not_quite_object::__Not_quite_object;

	/* function copy() 一般泛型 */
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  _STD output_iterator<const _STD iter_value_t<Iterator>&> OutIter>
	constexpr OutIter operator()(Iterator first, Sentinel last, OutIter result) const noexcept
	{
		auto check_first = __unwrap_iterator<Sentinel>(__move(first));
		auto check_last	 = __get_last_iterator_unwrapped<Iterator, Sentinel>(check_first, __move(last));

		return __default_cpoy(__move(check_first), __move(check_last), __move(result));
	}

	/* function copy() for 容器 强化版 */
	template <_RANGES input_range Range, _STD weakly_incrementable OutIter>
	constexpr OutIter operator()(Range&& rng1, OutIter result) const noexcept
	{
		return __default_copy(ubegin(rng1), uend(rng1), result);
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
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Predicate  = _STD  plus<>,
			  typename Projection = _STD identity>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr Type
		operator()(Iterator first, Sentinel last, Type init, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		return __default_accumulate(_STD move(ufirst),
									_STD move(ulast),
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
		using diff_t = _STD iter_difference_t<Iterator>;

		diff_t n = 0;
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
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES equal_to,
			  typename Projection = _STD   identity>
		requires(_STD indirect_binary_predicate<Predicate, _STD projected<Iterator, Projection>, const Type*>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _STD iter_difference_t<Iterator>
		operator()(Iterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		return __default_count(__unwrap_iterator<Sentinel>(_STD move(first)),
							   __unwrap_sentinel<Iterator>(_STD move(last)),
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
		return __default_count(ubegin(rng), uend(rng), _STD move(value), pred, proj);
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
			*(first++) = (value++);
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function itoa() 标准版 */
	template <_STD forward_iterator Iterator, _STD sentinel_for<Iterator> Sentinel, typename Type>
	constexpr void operator()(Iterator first, Sentinel last, const Type& value) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		__default_itoa(_STD move(ufirst), _STD move(ulast), _STD move(value));
	}

	/* function itoa() for 容器 加强版 */
	template <_RANGES forward_range Range, typename Type>
	constexpr auto operator()(Range&& rng, const Type& value) const noexcept
	{
		__default_itoa(ubegin(rng), uend(rng), _STD move(value));
	}
};

constexpr inline __Itoa_function itoa { __Not_quite_object::__Construct_tag {} };

// 在 [first, last) 范围内查找第一个与 value 相等的元素。可指定投影 proj ，不可指定谓词 pred
struct __Find_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Type, typename Projection>
	_NODISCARD static constexpr Iterator
		__default_find(Iterator first, Iterator last, const Type& value, Projection proj) noexcept
	{
		while ((first != last) && ((_STD invoke(proj, *first)) == value))
		{
			++first;
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function find() for 仿函数 标准版 */
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Projection = _STD identity>
		requires(_STD indirect_binary_predicate<_RANGES equal_to, _STD projected<Iterator, Projection>, const Type*>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator
		operator()(Iterator first, Sentinel last, const Type& value, Projection proj = {}) const noexcept
	{
		auto result = __default_find(__unwrap_iterator<Sentinel>(_STD move(first)),
									 __unwrap_sentinel<Iterator>(_STD move(last)),
									 _STD move(value),
									 __check_function(proj));

		__seek_wrapped(first, _STD move(result));

		return first;
	}

	/* function find() for 仿函数、容器 强化版 */
	template <_RANGES input_range Range, typename Type, typename Projection = _STD identity>
		requires(_STD indirect_binary_predicate<_RANGES equal_to,
												_STD	projected<_RANGES iterator_t<Range>, Projection>,
												const Type*>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto result = __default_find(__unwrap_range_iterator<Range>(_STD move(first)),
									 uend(rng),
									 _STD move(value),
									 __check_function(proj));

		__seek_wrapped(first, _STD move(result));

		return first;
	}
};

constexpr inline __Find_function find { __Not_quite_object::__Construct_tag {} };

// 在 [first, last) 范围内查找第一个与 value 相等的元素。可指定谓词 pred 与投影 proj
struct _find_if_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator
		__default_find_if(Iterator first, Iterator last, const Type& value, Predicate pred, Projection proj) noexcept
	{
		while ((first != last) && _STD invoke(pred, value, _STD invoke(proj, *first)))
		{
			++first;
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function find_if() for 仿函数 标准版 */
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Predicate  = _STD  identity,
			  typename Projection = _STD identity>
		requires(_STD indirect_unary_predicate<Predicate, _STD projected<Iterator, Projection>>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator operator()(Iterator	first,
																				Sentinel	last,
																				const Type& value,
																				Predicate	pred,
																				Projection	proj = {}) const noexcept
	{
		auto result = __default_find_if(__unwrap_iterator<Sentinel>(_STD move(first)),
										__unwrap_sentinel<Iterator>(_STD move(last)),
										_STD move(value),
										__check_function(pred),
										__check_function(proj));

		__seek_wrapped(first, _STD move(result));

		return first;
	}

	/* function find_if() for 容器 强化版 */
	template <_RANGES input_range Range,
			  typename Type,
			  typename Predicate  = _STD  identity,
			  typename Projection = _STD identity>
		requires(_STD indirect_unary_predicate<Predicate, _STD projected<_RANGES iterator_t<Range>, Projection>>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto result = __default_find_if(__unwrap_range_iterator<Range>(_STD move(first)),
										uend(rng),
										_STD move(value),
										__check_function(pred),
										__check_function(proj));

		__seek_wrapped(first, _STD move(result));

		return first;
	}
};

constexpr inline _find_if_function find_if { __Not_quite_object::__Construct_tag {} };

// 在 [first, last) 范围内查找第一个与 value 不相等的元素。可指定谓词 pred 与投影 proj
struct __Find_if_not_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD static constexpr Iterator __default_find_if_not(Iterator	   first,
															   Iterator	   last,
															   const Type& value,
															   Predicate   pred,
															   Projection  proj) noexcept
	{
		while ((first != last) && !(_STD invoke(pred, value, _STD invoke(proj, *first))))
		{
			++first;
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function find_if_not() for 仿函数 标准版 */
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Predicate,
			  typename Projection = _STD identity>
		requires(_STD indirect_unary_predicate<Predicate, _STD projected<Iterator, Projection>>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator operator()(Iterator	first,
																				Sentinel	last,
																				const Type& value,
																				Predicate	pred,
																				Projection	proj = {}) const noexcept
	{
		auto result = __default_find_if_not(__unwrap_iterator<Sentinel>(_STD move(first)),
											__unwrap_sentinel<Iterator>(_STD move(last)),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		__seek_wrapped(first, _STD move(result));

		return first;
	}

	/* function find_if_not() for 容器 强化版 */
	template <_RANGES input_range Range, typename Type, typename Predicate, typename Projection = _STD identity>
		requires(_STD indirect_unary_predicate<Predicate, _STD projected<_RANGES iterator_t<Range>, Projection>>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto result = __default_find_if_not(__unwrap_range_iterator<Range>(_STD move(first)),
											uend(rng),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		__seek_wrapped(first, _STD move(result));

		return first;
	}
};

constexpr inline __Find_if_not_function find_if_not { __Not_quite_object::__Construct_tag {} };

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
								_STD invoke(proj2, *i)))  // 如果序列 1 的元素与序列 2 中元素相等
				{
					return first1;
				}
			}
		}

		return first1;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function find_first_of() for 仿函数 标准版 */
	template <_STD input_iterator	Iterator1,
			  _STD forward_iterator Iterator2,
			  _STD sentinel_for<Iterator1> Sentinel1,
			  _STD sentinel_for<Iterator2> Sentinel2,
			  typename Predicate   = _RANGES equal_to,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD indirectly_comparable<Iterator1, Iterator2, Predicate, Projection1, Projection2>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator1 operator()(Iterator1	 first1,
																				 Sentinel1	 last1,
																				 Iterator2	 first2,
																				 Sentinel2	 last2,
																				 Predicate	 pred  = {},
																				 Projection1 proj1 = {},
																				 Projection2 proj2 = {}) const noexcept
	{
		auto ufirst1 = __unwrap_iterator<Sentinel1>(_STD move(first1));
		auto ufirst2 = __unwrap_iterator<Sentinel2>(_STD move(first2));
		auto ulast1	 = __get_last_iterator_with_unwrapped<Iterator1>(ufirst1, _STD move(last1));
		auto ulast2	 = __get_last_iterator_with_unwrapped<Iterator2>(ufirst2, _STD move(last2));

		auto result = __default_find_first_of(_STD move(ufirst1),
											  _STD move(ulast1),
											  _STD move(ufirst2),
											  _STD move(ulast2),
											  __check_function(pred),
											  __check_function(proj1),
											  __check_function(proj2));

		__seek_wrapped(first1, _STD move(result));

		return first1;
	}

	/* function find_first_of() for 容器、仿函数 强化版 */
	template <_RANGES input_range		   Range1,
			  _RANGES forward_range		   Range2,
			  typename Predicate   = _RANGES equal_to,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD indirectly_comparable<_RANGES iterator_t<Range1>,
											_RANGES iterator_t<Range2>,
											Predicate,
											Projection1,
											Projection2>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range1>
		operator()(Range1&& rng1, Range2&& rng2, Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {})
			const noexcept
	{
		auto first1 = _RANGES begin(rng1);

		auto result = __default_find_first_of(__unwrap_range_iterator<Range1>(_STD move(first1)),
											  uend(rng1),
											  ubegin(rng2),
											  uend(rng2),
											  __check_function(pred),
											  __check_function(proj1),
											  __check_function(proj2));

		__seek_wrapped(first1, _STD move(result));

		return first1;
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
		Type tmp = _STD move(static_cast<Type&&>(a));
		a		 = _STD		   move(static_cast<Type&&>(b));
		b		 = _STD		   move(static_cast<Type&&>(tmp));
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
		Iterator1 tmp = _STD move(a);
		*a			  = _STD			move(_RANGES iter_move(b));
		*b			  = _STD			move(_RANGES iter_move(tmp));
	}
};

constexpr inline __Iter_swap_function iter_swap { __Not_quite_object::__Construct_tag {} };

// 将 [first1, last1) 范围内的元素与 [first2, first2 + (last1 - first1)) 范围内的元素互相交换。不可指定谓词 pred 与投影 proj
struct __Swap_ranges_function: private __Not_quite_object
{
private:

	template <typename Iterator1, typename Iterator2>
	using swap_ranges_result = _RANGES in_in_result<Iterator1, Iterator2>;

	// 统一调用方式
	template <typename Iterator1, typename Sentinel1, typename Iterator2, typename Sentinel2>
	static constexpr swap_ranges_result<Iterator1, Iterator2>
		__default_swap_ranges(Iterator1 first1, Sentinel1 last1, Iterator2 first2, Sentinel2 last2) noexcept
	{
		for (; (first1 != last1) && (first2 != last2); ++first1, ++first2)
		{
			iter_swap(first1, first2);
		}

		return { _STD move(first1), _STD move(first2) };
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function swap_ranges() 标准版 */
	template <_STD input_iterator Iterator1,
			  _STD input_iterator Iterator2,
			  _STD sentinel_for<Iterator1> Sentinel1,
			  _STD sentinel_for<Iterator2> Sentinel2>
		requires(_STD indirectly_swappable<Iterator1, Iterator2>)
	constexpr swap_ranges_result<Iterator1, Iterator2>
		operator()(Iterator1 first1, Sentinel1 last1, Iterator2 first2, Sentinel2 last2) const noexcept
	{
		auto [ end_for_range1, end_for_range2 ] = __default_swap_ranges(__unwrap_iterator<Sentinel1>(_STD move(first1)),
																		__unwrap_sentinel<Iterator1>(_STD move(last1)),
																		__unwrap_iterator<Sentinel2>(_STD move(first2)),
																		__unwrap_sentinel<Iterator2>(_STD move(last2)));

		__seek_wrapped(first1, _STD move(end_for_range1));
		__seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2) };
	}

	/* function swap_ranges() for 容器 强化版 */
	template <_RANGES input_range Range1, _RANGES input_range Range2>
		requires(_STD indirectly_swappable<_RANGES iterator_t<Range1>, _RANGES iterator_t<Range2>>)
	constexpr swap_ranges_result<_RANGES borrowed_iterator_t<Range1>, _RANGES borrowed_iterator_t<Range2>>
		operator()(Range1&& rng1, Range2&& rng2) const noexcept
	{
		auto first1 = _RANGES begin(rng1);
		auto first2 = _RANGES begin(rng2);

		auto [ end_for_range1, end_for_range2 ] =
			__default_swap_ranges(__unwrap_range_iterator<Range1>(_STD move(first1)),
								  uend(rng1),
								  __unwrap_range_iterator<Range2>(_STD move(first2)),
								  uend(rng2));

		__seek_wrapped(first1, _STD move(end_for_range1));
		__seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2) };
	}
};

constexpr inline __Swap_ranges_function swap_ranges { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素依次调用 pred 。必须指定谓词 pred ，可指定投影 proj
struct __For_each_function: private __Not_quite_object
{
private:

	template <class Iterator, class Predicate>
	using for_each_result = _RANGES in_fun_result<Iterator, Predicate>;

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr for_each_result<Iterator, Predicate>
		__default_for_each(Iterator first, Iterator last, Predicate pred, Projection proj) noexcept
	{
		for (; first != last; ++first)
		{
			_STD invoke(pred, _STD invoke(proj, *first));
		}

		return { _STD move(first), _STD move(pred) };
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function for_each() 标准版 */
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Projection = _STD  identity,
			  _STD indirectly_unary_invocable<_STD projected<Iterator, Projection>> Predicate>
		requires(requires(Iterator iter, Predicate pred, Projection proj) {
			_STD invoke(pred, _STD invoke(proj, _RANGES iter_move(iter)));
		})
	constexpr for_each_result<Iterator, Predicate>
		operator()(Iterator first, Sentinel last, Predicate pred, Projection proj = {}) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		auto [ end_for_range, upred ] =
			__default_for_each(_STD move(ufirst), _STD move(ulast), __check_function(pred), __check_function(proj));

		__seek_wrapped(first, _STD move(end_for_range));

		return { _STD move(first), _STD move(upred) };
	}

	/* function for_each() for 容器 强化版 */
	template <_RANGES input_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirectly_unary_invocable<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
			_STD invoke(pred, _STD invoke(proj, _RANGES iter_move(_RANGES begin(rng))));
		})
	inline for_each_result<_RANGES borrowed_iterator_t<Range>, Predicate>
		operator()(Range&& rng, Predicate pred, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto [ end_for_range, upred ] = __default_for_each(__unwrap_range_iterator<Range>(_STD move(first)),
														   uend(rng),
														   _STD move(pred),
														   __check_function(proj));

		__seek_wrapped(first, _STD move(end_for_range));

		return { _STD move(first), _STD move(upred) };
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
		if ((last1 - first1) != (last2 - first2))  // 如果序列 1 的元素数量与序列 2 的元素数量不相等, 返回 false
		{
			return false;
		}

		for (; first1 != last1; ++first1, ++first2)	 // 如果序列 1 的元素数量多于序列 2 的元素数量，顺次比较
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
	template <_STD input_iterator Iterator1,
			  _STD input_iterator Iterator2,
			  _STD sentinel_for<Iterator1> Sentinel1,
			  _STD sentinel_for<Iterator2> Sentinel2,
			  typename Predicate   = _RANGES equal_to,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(requires(Iterator1 first1, Iterator2 first2, Predicate pred, Projection1 proj1, Projection2 proj2) {
			{
				_STD invoke(pred,
							_STD invoke(proj1, _RANGES iter_move(first1)),
							_STD invoke(proj2, _RANGES iter_move(first2)))
			} -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr bool operator()(Iterator1	first1,
																			Sentinel1	last1,
																			Iterator2	first2,
																			Sentinel2	last2,
																			Predicate	pred  = {},
																			Projection1 proj1 = {},
																			Projection2 proj2 = {}) const noexcept
	{
		auto ufirst1 = __unwrap_iterator<Sentinel1>(_STD move(first1));
		auto ulast1	 = __unwrap_sentinel<Iterator1>(_STD move(last1));

		auto ufirst2 = __unwrap_iterator<Sentinel2>(_STD move(first2));
		auto ulast2	 = __unwrap_sentinel<Iterator2>(_STD move(last2));

		return __default_equal(_STD move(ufirst1),
							   _STD move(ulast1),
							   _STD move(ufirst2),
							   _STD move(ulast2),
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
							_STD invoke(proj1, _RANGES iter_move(_RANGES begin(rng1))),
							_STD invoke(proj2, _RANGES iter_move(_RANGES begin(rng2))))
			} -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto operator()(Range&&		rng1,
																			Range&&		rng2,
																			Predicate	pred  = {},
																			Projection1 proj1 = {},
																			Projection2 proj2 = {}) const noexcept
	{
		return __default_equal(ubegin(rng1), uend(rng1), ubegin(rng2), uend(rng2), pred, proj1, proj2);
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
	template <_STD forward_iterator Iterator, _STD sentinel_for<Iterator> Sentinel, typename Type>
	constexpr void operator()(Iterator first, Sentinel last, const Type& value) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __unwrap_sentinel<Iterator>(_STD move(last));

		__default_fill(_STD move(ufirst), _STD move(ulast), _STD move(value));
	};

	/* function fill() for 容器 强化版 */
	template <_RANGES forward_range Range, typename Type>
	constexpr auto operator()(Range&& rng, const Type& value) const noexcept
	{
		__default_fill(ubegin(rng), uend(rng), _STD move(value));
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
	template <typename Type, _STD output_iterator<const Type&> Iterator>
	constexpr Iterator operator()(Iterator first, _STD iter_difference_t<Iterator> n, const Type& value) const noexcept
	{
		return __default_fill_n(_STD move(first), _STD move(n), _STD move(value));
	};

	/* function fill_n() for 容器 强化版 */
	template <typename Type, _RANGES output_range<const Type&> Range>
	constexpr auto operator()(Range&& rng, _RANGES range_difference_t<Range> n, const Type& value) const noexcept
	{
		return __default_fill_n(ubegin(rng), _STD move(n), _STD move(value));
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
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))),
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))))
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
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))),
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))))
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
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))),
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))))
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
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))),
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))))
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
	template <_STD forward_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Projection															  = _STD  identity,
			  _STD indirect_strict_weak_order<_STD projected<Iterator, Projection>> Predicate = _RANGES less>
		requires(requires(Iterator first, Predicate pred, Projection proj) {
			{
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(first)),
							_STD invoke(proj, _RANGES iter_move(first)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator
		operator()(Iterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return __seek_wrapped(first,
							  __default_max_element(__unwrap_iterator<Sentinel>(_STD move(first)),
													__unwrap_sentinel<Iterator, Sentinel>(_STD move(last)),
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
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))),
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))))
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
	template <_STD forward_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Projection															  = _STD  identity,
			  _STD indirect_strict_weak_order<_STD projected<Iterator, Projection>> Predicate = _RANGES greater>
		requires(requires(Iterator first, Predicate pred, Projection proj) {
			{
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(first)),
							_STD invoke(proj, _RANGES iter_move(first)))
			} noexcept -> _STD convertible_to<bool>;
		})
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator
		operator()(Iterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return __seek_wrapped(first,
							  __default_min_element(__unwrap_iterator<Sentinel>(_STD move(first)),
													__unwrap_sentinel<Iterator, Sentinel>(_STD move(last)),
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
				_STD invoke(pred,
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))),
							_STD invoke(proj, _RANGES iter_move(ubegin(rng))))
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

	template <typename Iterator1, typename Iterator2, typename Iterator>
	using merge_result = _RANGES in_in_out_result<Iterator1, Iterator2, Iterator>;

	// 统一调用方式
	template <typename Iterator1,
			  typename Iterator2,
			  typename OutIter,
			  typename Predicate,
			  typename Projection1,
			  typename Projection2>
	_NODISCARD static constexpr merge_result<Iterator1, Iterator2, OutIter> __default_merge(Iterator1	first1,
																							Iterator1	last1,
																							Iterator2	first2,
																							Iterator2	last2,
																							OutIter		result,
																							Predicate	pred,
																							Projection1 proj1,
																							Projection2 proj2) noexcept
	{
		while ((first1 != last1) && (first2 != last2))	// 若两个序列都未完成，则继续
		{
			if (_STD invoke(pred, _STD invoke(proj2, *first2), _STD invoke(proj1, *first1)))  // 若序列 2 的元素比较小
			{
				*result = *first2;	// 则记录序列 2 的元素
				++first2;			// 同时序列 2 前进 1 位
			}
			else  // 反之同理
			{
				*result = *first1;
				++first1;
			}

			++result;  // 每记录一个元素，result 前进 1 位
		}

		// 最后剩余元素拷贝到目的端。（两个序列一定至少有一个为空）
		if (first1 == last1)
		{
			auto [ end_for_range1, end_for_range_2 ] =
				_RANGES copy(_STD move(first2), _STD move(last2), _STD move(result));

			return { _STD move(first1), _STD move(end_for_range1), _STD move(end_for_range_2) };
		}
		else  // first2 == last2
		{
			auto [ end_for_range1, end_for_range_2 ] =
				_RANGES copy(_STD move(first1), _STD move(last1), _STD move(result));

			return { _STD move(end_for_range1), _STD move(first2), _STD move(end_for_range_2) };
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function merge() for 仿函数 标准版 */
	template <_STD input_iterator Iterator1,
			  _STD input_iterator Iterator2,
			  _STD sentinel_for<Iterator1> Sentinel1,
			  _STD sentinel_for<Iterator2> Sentinel2,
			  _STD weakly_incrementable	   OutIter,
			  typename Predicate   = _RANGES less,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD mergeable<Iterator1, Iterator2, OutIter, Predicate, Projection1, Projection2>)
	constexpr merge_result<Iterator1, Iterator2, OutIter> operator()(Iterator1	 first1,
																	 Sentinel1	 last1,
																	 Iterator2	 first2,
																	 Sentinel2	 last2,
																	 OutIter	 result,
																	 Predicate	 pred  = {},
																	 Projection1 proj1 = {},
																	 Projection2 proj2 = {}) const noexcept
	{
		auto [ end_for_range1, end_for_range2, end_for_range3 ] =
			__default_merge(__unwrap_iterator<Sentinel1>(_STD move(first1)),
							__unwrap_sentinel<Iterator1, Sentinel1>(_STD move(last1)),
							__unwrap_iterator<Sentinel2>(_STD move(first2)),
							__unwrap_sentinel<Iterator2, Sentinel2>(_STD move(last2)),
							_STD move(result),
							__check_function(pred),
							__check_function(proj1),
							__check_function(proj2));

		__seek_wrapped(first1, _STD move(end_for_range1));
		__seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2), _STD move(end_for_range3) };
	}

	/* function merge() for 容器、仿函数 强化版 */
	template <_RANGES input_range		   Range1,
			  _RANGES input_range		   Range2,
			  _STD weakly_incrementable	   OutIter,
			  typename Predicate   = _RANGES less,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD mergeable<_RANGES iterator_t<Range1>,
								_RANGES iterator_t<Range2>,
								OutIter,
								Predicate,
								Projection1,
								Projection2>)
	constexpr merge_result<_RANGES borrowed_iterator_t<Range1>, _RANGES borrowed_iterator_t<Range2>, OutIter>
		operator()(Range1&&	   rng1,
				   Range2&&	   rng2,
				   OutIter	   result,
				   Predicate   pred	 = {},
				   Projection1 proj1 = {},
				   Projection2 proj2 = {}) const noexcept
	{
		auto first1 = _RANGES begin(rng1);
		auto first2 = _RANGES begin(rng2);

		auto [ end_for_range1, end_for_range2, end_for_range3 ] =
			__default_merge(__unwrap_range_iterator<Range1>(_STD move(first1)),
							uend(rng1),
							__unwrap_range_iterator<Range2>(_STD move(first2)),
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

	template <typename Iterator, typename OutIter>
	using unary_transform_result = _RANGES in_out_result<Iterator, OutIter>;

	template <typename Iterator1, typename Iterator2, typename OutIter>
	using binary_transform_result = _RANGES in_in_out_result<Iterator1, Iterator2, OutIter>;

	// 统一调用方式 1
	template <typename Iterator, typename OutIter, typename Predicate, typename Projection>
	_NODISCARD static constexpr unary_transform_result<Iterator, OutIter>
		__default_transform(Iterator first, Iterator last, OutIter result, Predicate pred, Projection proj) noexcept
	{
		for (; first != last; ++first, (void)++result)
		{
			*result = _STD invoke(pred, _STD invoke(proj, *first));
		}

		return { _STD move(first), _STD move(result) };
	}

	// 统一调用方式 2
	template <typename Iterator1,
			  typename Sentinel1,
			  typename Iterator2,
			  typename Sentinel2,
			  typename OutIter,
			  typename Predicate,
			  typename Projection1,
			  typename Projection2>
	_NODISCARD static constexpr binary_transform_result<Iterator1, Iterator2, OutIter>
		__default_transform(Iterator1		first1,
							const Sentinel1 last1,
							Iterator2		first2,
							const Sentinel2 last2,
							OutIter			result,
							Predicate		pred,
							Projection1		proj1,
							Projection2		proj2)
	{
		for (; first1 != last1 && first2 != last2; ++first1, (void)++first2, ++result)
		{
			*result = _STD invoke(pred, _STD invoke(proj1, *first1), _STD invoke(proj2, *first2));
		}

		return { _STD move(first1), _STD move(first2), _STD move(result) };
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// function transform() for 仿函数 标准版
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  _STD weakly_incrementable	  OutIter,
			  _STD copy_constructible Predicate = _STD identity,
			  typename Projection				= _STD				 identity>
		requires(
			_STD indirectly_writable<OutIter, _STD indirect_result_t<Predicate&, _STD projected<Iterator, Projection>>>)
	constexpr unary_transform_result<Iterator, OutIter>
		operator()(Iterator first, Sentinel last, OutIter result, Predicate pred, Projection proj = {}) const noexcept
	{
		auto [ end_for_range1, end_for_range2 ] = __default_transform(__unwrap_iterator<Sentinel>(_STD move(first)),
																	  __unwrap_sentinel<Iterator>(_STD move(last)),
																	  _STD move(result),
																	  __check_function(pred),
																	  __check_function(proj));

		__seek_wrapped(first, _STD move(end_for_range1));

		return { _STD move(first), _STD move(end_for_range2) };
	}

	// function transform() for 容器 强化版
	template <_RANGES input_range		Range,
			  _STD weakly_incrementable OutIter,
			  _STD copy_constructible Predicate = _STD identity,
			  typename Projection				= _STD				 identity>
		requires(_STD indirectly_writable<
				 OutIter,
				 _STD indirect_result_t<Predicate&, _STD projected<_RANGES iterator_t<Range>, Projection>>>)
	constexpr unary_transform_result<_RANGES borrowed_iterator_t<Range>, OutIter>
		operator()(Range&& rng, OutIter result, Predicate pred, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto [ end_for_range1, end_for_range2 ] = __default_transform(__unwrap_range_iterator<Range>(_STD move(first)),
																	  uend(rng),
																	  _STD move(result),
																	  __check_function(pred),
																	  __check_function(proj));

		__seek_wrapped(first, _STD move(end_for_range1));

		return { _STD move(first), _STD move(end_for_range2) };
	}

	// function transform() for 仿函数 标准版 2
	template <_STD input_iterator Iterator1,
			  _STD input_iterator Iterator2,
			  _STD sentinel_for<Iterator1> Sentinle1,
			  _STD sentinel_for<Iterator2> Sentinel2,
			  _STD weakly_incrementable	   OutIter,
			  _STD copy_constructible	   Predicate,
			  typename Projection1 = _STD  identity,
			  typename Projection2 = _STD  identity>
		requires(_STD indirectly_writable<OutIter,
										  _STD indirect_result_t<Predicate&,
																 _STD projected<Iterator1, Projection1>,
																 _STD projected<Iterator2, Projection2>>>)
	constexpr binary_transform_result<Iterator1, Iterator2, OutIter> operator()(Iterator1	first1,
																				Sentinle1	last1,
																				Iterator2	first2,
																				Sentinel2	last2,
																				OutIter		result,
																				Predicate	pred,
																				Projection1 proj1 = {},
																				Projection2 proj2 = {}) const
	{
		auto [ end_for_range1, end_for_range2, end_for_range3 ] =
			__default_transform(__unwrap_iterator<Sentinle1>(_STD move(first1)),
								__unwrap_sentinel<Iterator1>(_STD move(last1)),
								__unwrap_iterator<Sentinel2>(_STD move(first2)),
								__unwrap_sentinel<Iterator2>(_STD move(last2)),
								_STD move(result),
								__check_function(pred),
								__check_function(proj1),
								__check_function(proj2));

		__seek_wrapped(first1, _STD move(end_for_range1));
		__seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2), _STD move(end_for_range3) };
	}

	// function transform() for 容器 强化版 2
	template <_RANGES input_range		Range1,
			  _RANGES input_range		Range2,
			  _STD weakly_incrementable OutIter,
			  _STD copy_constructible	Predicate,
			  class Projection1 = _STD	identity,
			  class Projection2 = _STD	identity>
		requires(
			_STD indirectly_writable<OutIter,
									 _STD indirect_result_t<Predicate&,
															_STD projected<_RANGES iterator_t<Range1>, Projection1>,
															_STD projected<_RANGES iterator_t<Range2>, Projection2>>>)
	constexpr binary_transform_result<_RANGES borrowed_iterator_t<Range1>, _RANGES borrowed_iterator_t<Range2>, OutIter>
		operator()(Range1&&	   rng1,
				   Range2&&	   rng2,
				   OutIter	   result,
				   Predicate   pred,
				   Projection1 proj1 = {},
				   Projection2 proj2 = {}) const
	{
		auto first1 = _RANGES begin(rng1);
		auto first2 = _RANGES begin(rng2);

		auto [ end_for_range1, end_for_range2, end_for_range3 ] =
			__default_transform(__unwrap_range_iterator<Range1>(_STD move(first1)),
								uend(rng1),
								__unwrap_range_iterator<Range2>(_STD move(first2)),
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
		if (first == middle)  // 如果序列 1 为空，则直接返回
		{
			return;
		}

		_RANGES make_heap(first, middle, pred, proj);

		for (Iterator next { middle }; next != last; ++next)
		{
			if (_STD invoke(pred, _STD invoke(proj, *next), _STD invoke(proj, *first)))	 // 如果序列 2 的元素比较小
			{
				_RANGES _Pop_heap_hole_unchecked(first, middle, next, _RANGES iter_move(next), pred, proj, proj);
			}
		}

		_RANGES sort_heap(_STD move(first), _STD move(middle), pred, proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function partial_sort() for 仿函数 标准版 */
	template <_STD random_access_iterator Iterator,
			  _STD sentinel_for<Iterator>  Sentinel,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_sort_function<Iterator, Predicate, Projection>)
	constexpr void operator()(Iterator first, Iterator middle, Sentinel last, Predicate pred = {}, Projection proj = {})
		const noexcept
	{
		if constexpr (_STD is_same_v<Iterator, Sentinel>)
		{
			__default_partial_sort(__unwrap_iterator<Sentinel>(_STD move(first)),
								   __unwrap_iterator<Sentinel>(_STD move(middle)),
								   __unwrap_sentinel<Iterator>(last),
								   __check_function(pred),
								   __check_function(proj));

			return last;
		}
		else
		{
			auto umiddle = __unwrap_iterator<Sentinel>(_STD move(middle));
			auto ulast	 = __get_last_iterator_with_unwrapped<Iterator>(umiddle, _STD move(last));

			__seek_wrapped(middle, ulast);

			__default_partial_sort(__unwrap_iterator<Sentinel>(_STD move(first)),
								   _STD move(umiddle),
								   _STD move(ulast),
								   __check_function(pred),
								   __check_function(proj));

			return middle;
		}
	}

	/* function partial_sort() for 容器、仿函数 强化版 */
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range> operator()(Range&& rng,
															_RANGES iterator_t<Range> middle,
															Predicate				  pred = {},
															Projection				  proj = {}) const noexcept
	{
		if constexpr (_RANGES common_range<Range>)
		{
			__default_partial_sort(ubegin(rng),
								   __unwrap_range_iterator<Range>(_STD move(middle)),
								   uend(rng),
								   __check_function(pred),
								   __check_function(proj));

			return _RANGES end(rng);
		}
		else
		{
			auto umiddle = __unwrap_range_iterator<Range>(_STD move(middle));
			auto ulast	 = __get_last_iterator_with_unwrapped(rng, umiddle);

			__seek_wrapped(middle, ulast);

			__default_partial_sort(ubegin(rng),
								   _STD move(umiddle),
								   _STD move(ulast),
								   __check_function(pred),
								   __check_function(proj));

			return middle;
		}
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

		using value_t = _STD iter_value_t<Iterator>;

		for (Iterator i { first }; ++i != last;)
		{
			value_t value { _RANGES iter_move(i) };

			Iterator poivt { i };

			for (Iterator j { poivt }; poivt != first;)
			{
				--j;

				if (!(_STD invoke(pred, _STD invoke(proj, value), _STD invoke(proj, *j))))
				{
					break;
				}

				*poivt = _RANGES iter_move(j);

				--poivt;
			}

			*poivt = _STD move(value);
		}

		return last;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// insertion_sort() for 仿函数 标准版
	template <_STD bidirectional_iterator Iterator,
			  _STD sentinel_for<Iterator>  Sentinel,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_sort_function<Iterator, Predicate, Projection>)
	constexpr Iterator
		operator()(Iterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		const auto lenth = ulast - ufirst;

		ufirst = __default_insertion_sort(_STD move(ufirst),
										  _STD move(ulast),
										  __check_function(pred),
										  __check_function(proj));

		__seek_wrapped(first, ufirst);

		return first;
	}

	// insertion_sort()	for 容器、仿函数 强化版
	template <_RANGES bidirectional_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto result = __default_insertion_sort(ubegin(rng), uend(rng), pred, proj);

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
struct __Set_median_of_three_function: private __Not_quite_object
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

	template <_STD random_access_iterator Iterator, typename Predicate, typename Projection>
	constexpr void operator()(__zh_No_inspection_required_function,
							  Iterator	 first,
							  Iterator	 middle,
							  Iterator	 last,
							  Predicate	 pred,
							  Projection proj) const noexcept
	{
		using diff_t = _STD iter_difference_t<Iterator>;

		const diff_t lenth = last - first;

		if ((__max_get_median_of_three_constant<diff_t>) < lenth)
		{
			const diff_t step1 = (lenth + 1) >> 3;	// +1 不会溢出，因为在调用方中使范围包含在内
			const diff_t step2 = step1 << 1;		// 注意：有意丢弃低位

			__default_set_median_of_three(first, _RANGES next(first, step1), _RANGES next(first, step2), pred, proj);

			__default_set_median_of_three(_RANGES next(middle, -step1),
										  middle,
										  _RANGES next(middle, step1),
										  pred,
										  proj);

			__default_set_median_of_three(_RANGES next(last, -step2), _RANGES next(last, -step1), last, pred, proj);

			__default_set_median_of_three(_RANGES next(first, step1), middle, _RANGES next(last, -step1), pred, proj);
		}
		else
		{
			__default_set_median_of_three(first, middle, last, pred, proj);
		}
	}
};

constexpr inline __Set_median_of_three_function set_median_of_three { __Not_quite_object::__Construct_tag {} };

// 此处实现 partition() 进行无边界检查的 分割序列 。原本服务于 quick_sort() ，单独写出是为了在其他地方调用
struct __Partition_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	// 用于分割序列，选择一个枢轴，将[first，last）划分为小于枢轴的元素、等于枢轴的元素和大于枢轴的元素；将相等的分区作为子范围返回。
	template <_STD random_access_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Predicate,
			  typename Projection>
	constexpr _RANGES subrange<Iterator> operator()(__zh_No_inspection_required_function,
													Iterator   first,
													Sentinel   last,
													Predicate  pred,
													Projection proj) const noexcept
	{
		Iterator mid = first + ((last - first) >> 1);

		set_median_of_three(__zh_No_inspection_required, first, mid, _RANGES prev(last), pred, proj);

		Iterator pfirst = mid;
		Iterator plast	= _RANGES next(pfirst);

		while ((first < pfirst)
			   && (!(_STD invoke(pred, _STD invoke(proj, *(_RANGES prev(pfirst))), _STD invoke(proj, *pfirst))))
			   && (!(_STD invoke(pred, _STD invoke(proj, *pfirst), _STD invoke(proj, *(_RANGES prev(pfirst)))))))
		{
			--pfirst;
		}

		while ((plast < last) && (!(_STD invoke(pred, _STD invoke(proj, *plast), _STD invoke(proj, *pfirst))))
			   && (!(_STD invoke(pred, _STD invoke(proj, *pfirst), _STD invoke(proj, *plast)))))
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
};

constexpr inline __Partition_function partition { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素快速排序。可指定谓词 pred 与投影 proj
struct __Quick_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void __default_quick_sort(Iterator first,
											   Iterator last,
											   _STD iter_difference_t<Iterator> lenth,
											   Predicate						pred,
											   Projection						proj) noexcept
	{
		while (1 < lenth)
		{
			auto [ pfirst, plast ] = partition(__zh_No_inspection_required, first, last, pred, proj);

			if ((pfirst - first) < (last - plast))
			{
				__default_quick_sort(first, pfirst, _RANGES distance(first, pfirst), pred, proj);

				first = _STD move(plast);
			}
			else
			{
				__default_quick_sort(plast, last, _RANGES distance(plast, last), pred, proj);

				last = _STD move(pfirst);
			}
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// quick_sort() for 仿函数 标准版
	template <_STD random_access_iterator Iterator,
			  _STD sentinel_for<Iterator>  Sentinel,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_sort_function<Iterator, Predicate, Projection>)
	constexpr Iterator
		operator()(Iterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		__seek_wrapped(first, ufirst);

		const auto lenth = ulast - ufirst;

		__default_quick_sort(_STD move(ufirst),
							 _STD move(ulast),
							 _STD move(lenth),
							 __check_function(pred),
							 __check_function(proj));

		return first;
	}

	// quick_sort() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto	   ufirst = ubegin(rng);
		auto	   ulast  = __get_last_iterator_with_unwrapped(rng);
		const auto lenth  = ulast - ufirst;

		__default_sort(_STD move(ufirst), ulast, lenth, pred, proj);

		return __rewrap_iterator(rng, _STD move(ufirst));
	}
};

constexpr inline __Quick_sort_function quick_sort { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素归并排序。可指定谓词 pred 与投影 proj
struct __Merge_sort_function: private __Not_quite_object
{
private:

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr void __default_merge_sort(Iterator first,
											   Iterator last,
											   _STD iter_difference_t<Iterator> lenth,
											   Predicate						pred,
											   Projection						proj) noexcept
	{
		using diff_t = _STD iter_difference_t<Iterator>;

		if ((lenth == 0) || (lenth == 1))
		{
			return;
		}

		Iterator middle = _RANGES next(first, static_cast<diff_t>(lenth >> 1));

		__default_merge_sort(first, middle, pred, proj);
		__default_merge_sort(middle, last, pred, proj);

		// TODO: 以期实现自己的 inplace_merge() ，同时，此前提到插入排序的缺点之一 “借助额外内存” ，就体现在此函数中
		_RANGES inplace_merge(_STD move(first), _STD move(middle), _STD move(last), pred, proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/*
	 * 归并排序 未被 sort 采纳，主要是考虑到：
	 * 1、需要借助额外的内存
	 * 2、在内存之间移动（复制）数据耗时不少
	 */

	// merge_sort() for 仿函数 强化版
	template <_STD bidirectional_iterator Iterator,
			  _STD sentinel_for<Iterator>  Sentinel,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_sort_function<Iterator, Predicate, Projection>)
	constexpr Iterator
		operator()(Iterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		__seek_wrapped(first, ufirst);

		const auto lenth = ulast - ufirst;

		__default_merge_sort(_STD move(ufirst),
							 _STD move(ulast),
							 _STD move(lenth),
							 __check_function(pred),
							 __check_function(proj));

		return first;
	}

	// merge_sort() for 容器、仿函数 强化版
	template <_RANGES bidirectional_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto	   ufirst = ubegin(rng);
		auto	   ulast  = __get_last_iterator_with_unwrapped(rng);
		const auto lenth  = ulast - ufirst;

		__default_merge_sort(_STD move(ufirst), ulast, lenth, pred, proj);

		return __rewrap_iterator(rng, _STD move(ufirst));
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
			if ((last - first)
				< (__stl_threshold<_STD iter_difference_t<Iterator>>))	// 使用插入排序，如果元素数量足够少
			{
				insertion_sort(__zh_No_inspection_required, _STD move(first), _STD move(last), pred, proj);

				return;
			}

			if (ideal == 0)	 // 使用堆排序，如果递归深度足够深
			{
				partial_sort(__zh_No_inspection_required, _STD move(first), last, _STD move(last), pred, proj);

				return;
			}

			ideal = (ideal >> 1) + (ideal >> 2);  // allow 1.5 log2(N) divisions

			// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
			auto [ pfirst, plast ] = partition(__zh_No_inspection_required, first, last, pred, proj);

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
	template <_STD random_access_iterator Iterator,
			  _STD sentinel_for<Iterator>  Sentinel,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_sort_function<Iterator, Predicate, Projection>)
	constexpr inline Iterator
		operator()(Iterator first, Sentinel last, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		__seek_wrapped(first, ufirst);

		const auto lenth = ulast - ufirst;

		__default_sort(_STD move(ufirst),
					   _STD move(ulast),
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
		auto	   ufirst = ubegin(rng);
		auto	   ulast  = __get_last_iterator_with_unwrapped(rng);
		const auto lenth  = ulast - ufirst;

		__default_sort(_STD move(ufirst), ulast, lenth, pred, proj);

		return __rewrap_iterator(rng, _STD move(ufirst));
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
		__default_nth_element(Iterator first, Iterator nth, Iterator last, Predicate pred, Projection proj) noexcept
	{
		while (3 < (last - first))
		{
			auto [ pfirst, plast ] = partition(__zh_No_inspection_required, first, last, pred, proj);

			if (nth < pfirst)  // 如果 指定位置 < 右段起点，（即 nth 位于右段）
			{
				first = _STD move(pfirst);	// 对右段实施分割
			}
			else  // 否则（nth 位于左段）
			{
				last = _STD move(plast);  // 对左段实施分割
			}
		}

		insertion_sort(__zh_No_inspection_required, _STD move(first), _STD move(last), pred, proj);
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function nth_element() for 仿函数 标准版 */
	template <_STD random_access_iterator Iterator,
			  _STD sentinel_for<Iterator>  Sentinel,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr Iterator operator()(Iterator	 first,
								  Iterator	 nth,
								  Sentinel	 last,
								  Predicate	 pred = {},
								  Projection proj = {}) const noexcept
	{
		auto unth  = __unwrap_iterator<Sentinel>(_STD move(nth));
		auto ulast = __get_last_iterator_with_unwrapped<Iterator>(unth, _STD move(last));

		__seek_wrapped(nth, ulast);

		__default_nth_element(_STD move(__unwrap_iterator<Sentinel>(_STD move(first))),
							  _STD move(unth),
							  _STD move(ulast),
							  __check_function(pred),
							  __check_function(proj));

		return nth;
	}

	/* function nth_element() for 容器、仿函数 强化版 */
	template <_RANGES random_access_range  Range,
			  _STD random_access_iterator  Iterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr auto operator()(Range&& rng, Iterator nth, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto unth  = __unwrap_range_iterator<Range>(_STD move(nth));
		auto ulast = [ & ]
		{
			if constexpr (_RANGES common_range<Range>)
			{
				return uend(rng);
			}
			else if constexpr (_RANGES sized_range<Range>)
			{
				return _RANGES next(ubegin(rng), _RANGES distance(rng));
			}
			else
			{
				return _RANGES next(unth, uend(rng));
			}
		}();

		__seek_wrapped(nth, ulast);

		__default_nth_element(ubegin(rng),
							  _STD move(unth),
							  _STD move(ulast),
							  __check_function(pred),
							  __check_function(proj));

		return nth;
	}
};

constexpr inline __Nth_element_function nth_element { __Not_quite_object::__Construct_tag {} };

/*------------------------------------------------------------------------------------------------*/



/* 此处实现一些关于二分查找的算法 */

template <typename Iterator, typename Type, typename Predicate, typename Projection>
concept __basic_concept_for_binary_search_function =
	requires(Iterator first, Iterator last, const Type& value, Predicate pred, Projection proj) {
		{
			_STD invoke(pred, _STD invoke(proj, _RANGES iter_move(first)), value)
		} noexcept -> _STD convertible_to<bool>;

		{
			_STD invoke(pred, value, _STD invoke(proj, _RANGES iter_move(first)))
		} noexcept -> _STD convertible_to<bool>;

		{
			_STD invoke(pred, _STD invoke(proj, _RANGES iter_move(first)), _STD invoke(proj, _RANGES iter_move(first)))
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
		using diff_t = _STD iter_difference_t<Iterator>;

		diff_t half { 0 };

		Iterator middle { first };

		while (0 < lenth)
		{
			half = lenth >> 1;

			middle = _RANGES next(first, half);

			if (_STD invoke(pred, _STD invoke(proj, *middle), value))
			{
				first = _STD move(middle);
				++first;

				lenth -= static_cast<diff_t>(half + 1);
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
	template <
		_STD forward_iterator Iterator,
		_STD sentinel_for<Iterator> Sentinel,
		typename Type,
		typename Projection																			 = _STD identity,
		_STD indirect_strict_weak_order<const Type*, _STD projected<Iterator, Projection>> Predicate = _RANGES less>
		requires(__basic_concept_for_binary_search_function<Iterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator operator()(Iterator	first,
																				Sentinel	last,
																				const Type& value,
																				Predicate	pred = {},
																				Projection	proj = {}) const noexcept
	{
		auto	   ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		const auto lenth  = _RANGES distance(ufirst, __unwrap_sentinel<Iterator, Sentinel>(_STD move(last)));

		ufirst = __default_lower_bound(_STD move(ufirst),
									   _STD move(lenth),
									   _STD move(value),
									   __check_function(pred),
									   __check_function(proj));

		__seek_wrapped(first, _STD move(ufirst));

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
		using diff_t = _STD iter_difference_t<Iterator>;

		diff_t half { 0 };

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

				lenth -= static_cast<diff_t>(half + 1);
			}
		}

		return first;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* upper_bound() for 仿函数 标准版 */
	template <_STD forward_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<Iterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr Iterator operator()(Iterator	first,
																				Sentinel	last,
																				const Type& value,
																				Predicate	pred = {},
																				Projection	proj = {}) const noexcept
	{
		auto	   ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		const auto lenth  = _RANGES distance(ufirst, __unwrap_sentinel<Iterator, Sentinel>(_STD move(last)));

		ufirst = __default_upper_bound(_STD move(ufirst),
									   _STD move(lenth),
									   _STD move(value),
									   __check_function(pred),
									   __check_function(proj));

		__seek_wrapped(first, _STD move(ufirst));

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
		using diff_t = _STD iter_difference_t<Iterator>;

		diff_t half { 0 };

		Iterator middle { first };

		while (0 < lenth)  // 如果整个区间尚未迭代完毕
		{
			half = lenth >> 1;	// 找出中间位置

			middle = _RANGES next(first, half);	 // 设定中央迭代器

			if (_STD invoke(pred, _STD invoke(proj, *middle), value))  // 如果 中央元素 < 指定值
			{
				first = _STD move(middle);
				++first;  // 将区间缩小（移至后半段），以提高效率

				lenth -= static_cast<diff_t>(half + 1);
			}
			else if (_STD invoke(pred, value, _STD invoke(proj, *middle)))	// 如果 中央元素 > 指定值
			{
				lenth = _STD move(half);  // 将区间缩小（移至前半段），以提高效率
			}
			else  // 如果 中央元素 == 指定值
			{
				// 在前半段寻找上限
				auto begin { _STD move(lower_bound(__zh_No_inspection_required, first, middle, value, pred, proj)) };

				// 在后半段寻找下限
				auto end { _STD move(upper_bound(__zh_No_inspection_required,
												 _STD move(++middle),
												 _STD move(_RANGES next(first, lenth)),
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
	template <_STD forward_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<Iterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES subrange<Iterator>
		operator()(Iterator first, Sentinel last, const Type& value, Predicate pred = {}, Projection proj = {})
			const noexcept
	{
		auto	   ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		const auto lenth  = __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last)) - ufirst;

		auto result = __default_equal_range(_STD move(ufirst),
											_STD move(lenth),
											_STD move(value),
											__check_function(pred),
											__check_function(proj));

		return __rewrap_subrange<_RANGES subrange<Iterator>>(first, _STD move(result));
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
		Iterator ufirst { lower_bound(__zh_No_inspection_required, first, last, value, pred, proj) };

		return ((ufirst != last) && !(_STD invoke(pred, value, _STD invoke(proj, *ufirst))));
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function binary_search() for 仿函数 标准版 */
	template <_STD forward_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<Iterator, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr bool operator()(Iterator	first,
																			Sentinel	last,
																			const Type& value,
																			Predicate	pred = {},
																			Projection	proj = {}) const noexcept
	{
		auto ufirst = __unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= __get_last_iterator_with_unwrapped<Iterator>(ufirst, _STD move(last));

		return __default_binary_search(_STD move(ufirst),
									   _STD move(ulast),
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
		return __default_binary_search(ubegin(rng),
									   uend(rng),
									   _STD move(value),
									   __check_function(pred),
									   __check_function(proj));
	}
};

constexpr inline __Binary_search_function binary_search { __Not_quite_object::__Construct_tag {} };

__END_NAMESPACE_ZHANG

#endif	// __HAS_CPP20
