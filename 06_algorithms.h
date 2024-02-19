#pragma once

#include "00_basicFile.h"


#ifdef _HAS_CXX20

_BEGIN_NAMESPACE_ZHANG

struct zh_No_inspection_required_function
{
	// 这是一个特别的标识符。如果某个函数具有未检查版本（即 不检查迭代器型别是否满足最低要求、
	// 不检查谓词是否满足要求、不检查投影是否满足要求、不检查是否满足自身的 requires （如果有））
	// 则在调用此函数的未检查版本时，第一个参数应当是 __zh_No_inspection_required，以示明确。
};

constexpr inline zh_No_inspection_required_function no_inspection_required {};

//
template <typename Result, typename Wrapped, typename Unwrapped>
_NODISCARD constexpr Result _rewrap_subrange(Wrapped& value, _RANGES subrange<Unwrapped>&& result)
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

//
template <_RANGES forward_range Range, typename Iterator>
_NODISCARD constexpr _RANGES iterator_t<Range> _rewrap_iterator(Range&& rng, Iterator&& iter)
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
concept _wrapped_seekable = requires(Iterator iter, UIterator uiter) { iter._Seek_to(_STD forward<UIterator>(uiter)); };

// 此函数的作用是将 uiter（通常是一个裸指针） 的信息，装载到 iter（通常是一个迭代器）
template <typename Iterator, typename UIterator>
constexpr void _seek_wrapped(Iterator& iter, UIterator&& uiter)
{
	if constexpr (_wrapped_seekable<Iterator, UIterator>)
	{
		iter._Seek_to(_STD forward<UIterator>(uiter));
	}
	else
	{
		iter = _STD forward<UIterator>(uiter);
	}

	return;
}

/*------------------------------------------------------------------------------------------------*/

/* 此处实现 copy()、copy_n()、copy_if()、copy_backward()、move()、move_backward()、move_if()、move_n()、move_if_n() 函数 */

// 此处实现 copy_if() 函数
struct __Copy_if_function
{
private:

	template <typename Iterator, typename OutIter>
	using copy_if_result = _RANGES in_out_result<Iterator, OutIter>;

	// 统一调用方式
	template <typename Iterator, typename Sentinel, typename OutIter, typename Predicate, typename Projection>
	_NODISCARD static constexpr copy_if_result<Iterator, OutIter>
		__default_copy_if(Iterator first, Sentinel last, OutIter result, Predicate pred, Projection proj) noexcept
	{
		for (; first != last; ++first)
		{
			if (_STD invoke(pred, _STD invoke(proj, *first)))
			{
				*result = *first;
				++result;
			}
		}

		return { _STD move(first), _STD move(result) };
	}

public:

	/* function copy_if() 一般泛型 */
	template <_STD input_iterator Iterator,
			  _STD sentinel_for<Iterator> Sentinel,
			  _STD weakly_incrementable	  OutIter,
			  typename Predicate,
			  typename Projection = _STD identity>
		requires(_STD indirectly_copyable<Iterator, OutIter>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr copy_if_result<Iterator, OutIter>
		operator()(Iterator first, Sentinel last, OutIter result, Predicate pred, Projection proj = {}) const noexcept
	{
		auto ufirst = _unwrap_iterator<Sentinel>(__move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, __move(last));

		auto [end_for_range1, end_for_range2] = __default_copy_if(__move(ufirst),
																  __move(ulast),
																  __move(result),
																  _check_function(pred),
																  __Check_fuunction(proj));

		_seek_wrapped(first, _STD move(end_for_range1));

		return { _STD move(first), _STD move(end_for_range2) };
	}

	/* function copy_if() for 容器 强化版 */
	template <_RANGES input_range		Range,
			  _STD weakly_incrementable OutIter,
			  typename Predicate,
			  typename Projection = _STD identity>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr auto operator()(Range&& rng, OutIter result, Predicate pred, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto [end_for_range1, end_for_range2] = __default_copy_if(_unwrap_range_iterator<Range>(_STD move(first)),
																  __uend(rng),
																  _STD move(result),
																  _check_function(pred),
																  _check_function(proj));

		_seek_wrapped(first, _STD move(end_for_range1));

		return { _STD move(first), _STD move(end_for_range2) };
	}
};

constexpr inline __Copy_if_function copy_if {};

// 此处实现 copy_n() 函数
struct __Copy_n_function
{
private:

	template <typename Iterator, typename OutIter>
	using copy_n_result = _RANGES in_out_result<Iterator, OutIter>;

	// 统一调用方式
	template <typename Iterator, typename Size, typename OutIter>
	_NODISCARD static constexpr copy_n_result<Iterator, OutIter>
		__default_copy_n(Iterator first, Size count, OutIter result) noexcept
	{
		for (; count > 0; --count, ++first, ++result)
		{
			*result = *first;
		}

		return { _STD move(first), _STD move(result) };
	}

public:

	/* function copy_n() 一般泛型 */
	template <_STD input_iterator Iterator, _STD weakly_incrementable OutIter>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr copy_n_result<Iterator, OutIter>
		operator()(Iterator first, _STD iter_difference_t<Iterator> count, OutIter result) const noexcept
	{
		return __default_copy_n(_STD move(first), _STD move(count), _STD move(result));
	}

	/* function copy_n() for 容器 强化版 */
	template <_RANGES input_range Range, _STD weakly_incrementable OutIter>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr auto
		operator()(Range&& rng, _STD iter_difference_t<_RANGES iterator_t<Range>> count, OutIter result) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto [end_for_range1, end_for_range2] = __default_copy_n(_STD move(first), _STD move(count), _STD move(result));

		_seek_wrapped(first, _STD move(end_for_range1));

		return { _STD move(first), _STD move(end_for_range2) };
	}
};

constexpr inline __Copy_n_function copy_n {};

//
struct __Move_function
{
private:

	template <typename Iterator, typename OutIter>
	using move_result = _RANGES in_out_result<Iterator, OutIter>;

	// 统一调用方式
	template <typename Iterator, typename Sentinel, typename OutIter>
	_NODISCARD static constexpr move_result<Iterator, OutIter>
		__default_move(Iterator first, Sentinel last, OutIter result) noexcept
	{
		for (; first != last; ++first, (void)++result)
		{
			*result = _RANGES iter_move(first);
		}

		return { _STD move(first), _STD move(result) };
	}

public:

	/* function move() 一般泛型 */
	template <_STD input_iterator Iterator, _STD sentinel_for<Iterator> Sentinel, _STD weakly_incrementable OutIter>
		requires(_STD indirectly_movable<Iterator, OutIter>)
	constexpr move_result<Iterator, OutIter> operator()(Iterator first, Sentinel last, OutIter result) const noexcept
	{
		auto [end_for_range1, end_for_range2] = __default_move(_unwrap_iterator<Sentinel>(_STD move(first)),
															   _unwrap_sentinel<Iterator>(_STD move(last)),
															   __move(result));

		_seek_wrapped(first, _STD move(end_for_range1));

		return { _STD move(first), _STD move(end_for_range2) };
	}

	/* function move() for 容器 强化版 */
	template <_RANGES input_range Range, _STD weakly_incrementable OutIter>
		requires(_STD indirectly_movable<_RANGES iterator_t<Range>, OutIter>)
	constexpr auto operator()(Range&& rng, OutIter result) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto [end_for_range1, end_for_range2] =
			__default_move(_unwrap_range_iterator<Range>(_STD move(first)), __uend(rng), _STD move(result));

		_seek_wrapped(first, _STD move(end_for_range1));

		return { _STD move(first), _STD move(end_for_range2) };
	}
};

constexpr inline __Move_function move {};

struct __Move_backward_function
{
private:

	_EXPORT_STD template <class _In, class _Out>
	using move_backward_result = _RANGES in_out_result<_In, _Out>;

	// concept-constrained for strict enforcement as it is used by several algorithms
	template <typename _It1, typename _It2>
	constexpr _It2 _Move_backward_common(const _It1 _First, _It1 _Last, _It2 _Result)
	{
		while (_First != _Last)
		{
			*--_Result = _RANGES iter_move(--_Last);
		}

		return _Result;
	}

public:

	template <_STD bidirectional_iterator _It1, _STD sentinel_for<_It1> _Se1, _STD bidirectional_iterator _It2>
		requires(_STD indirectly_movable<_It1, _It2>)
	constexpr move_backward_result<_It1, _It2> operator()(_It1 _First, _Se1 _Last, _It2 _Result) const
	{
		_Adl_verify_range(_First, _Last);
		auto _UFirst = _Unwrap_iter<_Se1>(_STD move(_First));
		auto _ULast	 = _Get_final_iterator_unwrapped<_It1>(_UFirst, _STD move(_Last));
		_Seek_wrapped(_First, _ULast);
		_Result = _RANGES _Move_backward_common(_STD move(_UFirst), _STD move(_ULast), _STD move(_Result));
		return { _STD move(_First), _STD move(_Result) };
	}

	template <_RANGES bidirectional_range _Rng, _STD bidirectional_iterator _It>
		requires(_STD indirectly_movable<_RANGES iterator_t<_Rng>, _It>)
	constexpr move_backward_result<_RANGES borrowed_iterator_t<_Rng>, _It> operator()(_Rng&& _Range, _It _Result) const
	{
		auto			  _ULast = _Get_final_iterator_unwrapped(_Range);
		_Result					 = _RANGES _Move_backward_common(_Ubegin(_Range), _ULast, _STD move(_Result));
		return { _Rewrap_iterator(_Range, _STD move(_ULast)), _STD move(_Result) };
	}
};

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
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));

		return __default_accumulate(_STD move(ufirst),
									_STD move(ulast),
									_STD move(init),
									_check_function(pred),
									_check_function(proj));
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
		return __default_count(_unwrap_iterator<Sentinel>(_STD move(first)),
							   _unwrap_sentinel<Iterator>(_STD move(last)),
							   _STD move(value),
							   _check_function(pred),
							   _check_function(proj));
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
		return __default_count(__ubegin(rng), __uend(rng), _STD move(value), pred, proj);
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

		return;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function itoa() 标准版 */
	template <_STD forward_iterator Iterator, _STD sentinel_for<Iterator> Sentinel, typename Type>
	constexpr void operator()(Iterator first, Sentinel last, const Type& value) const noexcept
	{
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));

		__default_itoa(_STD move(ufirst), _STD move(ulast), _STD move(value));

		return;
	}

	/* function itoa() for 容器 加强版 */
	template <_RANGES forward_range Range, typename Type>
	constexpr auto operator()(Range&& rng, const Type& value) const noexcept
	{
		__default_itoa(__ubegin(rng), __uend(rng), _STD move(value));

		return;
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
		auto result = __default_find(_unwrap_iterator<Sentinel>(_STD move(first)),
									 _unwrap_sentinel<Iterator>(_STD move(last)),
									 _STD move(value),
									 _check_function(proj));

		_seek_wrapped(first, _STD move(result));

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

		auto result = __default_find(_unwrap_range_iterator<Range>(_STD move(first)),
									 __uend(rng),
									 _STD move(value),
									 _check_function(proj));

		_seek_wrapped(first, _STD move(result));

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
		auto result = __default_find_if(_unwrap_iterator<Sentinel>(_STD move(first)),
										_unwrap_sentinel<Iterator>(_STD move(last)),
										_STD move(value),
										_check_function(pred),
										_check_function(proj));

		_seek_wrapped(first, _STD move(result));

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

		auto result = __default_find_if(_unwrap_range_iterator<Range>(_STD move(first)),
										__uend(rng),
										_STD move(value),
										_check_function(pred),
										_check_function(proj));

		_seek_wrapped(first, _STD move(result));

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
		auto result = __default_find_if_not(_unwrap_iterator<Sentinel>(_STD move(first)),
											_unwrap_sentinel<Iterator>(_STD move(last)),
											_STD move(value),
											_check_function(pred),
											_check_function(proj));

		_seek_wrapped(first, _STD move(result));

		return first;
	}

	/* function find_if_not() for 容器 强化版 */
	template <_RANGES input_range Range, typename Type, typename Predicate, typename Projection = _STD identity>
		requires(_STD indirect_unary_predicate<Predicate, _STD projected<_RANGES iterator_t<Range>, Projection>>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto result = __default_find_if_not(_unwrap_range_iterator<Range>(_STD move(first)),
											__uend(rng),
											_STD move(value),
											_check_function(pred),
											_check_function(proj));

		_seek_wrapped(first, _STD move(result));

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
		auto ufirst1 = _unwrap_iterator<Sentinel1>(_STD move(first1));
		auto ufirst2 = _unwrap_iterator<Sentinel2>(_STD move(first2));
		auto ulast1	 = _get_last_iterator_unwrapped<Iterator1>(ufirst1, _STD move(last1));
		auto ulast2	 = _get_last_iterator_unwrapped<Iterator2>(ufirst2, _STD move(last2));

		auto result = __default_find_first_of(_STD move(ufirst1),
											  _STD move(ulast1),
											  _STD move(ufirst2),
											  _STD move(ulast2),
											  _check_function(pred),
											  _check_function(proj1),
											  _check_function(proj2));

		_seek_wrapped(first1, _STD move(result));

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

		auto result = __default_find_first_of(_unwrap_range_iterator<Range1>(_STD move(first1)),
											  __uend(rng1),
											  __ubegin(rng2),
											  __uend(rng2),
											  _check_function(pred),
											  _check_function(proj1),
											  _check_function(proj2));

		_seek_wrapped(first1, _STD move(result));

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
		Type tmp = static_cast<Type&&>(a);
		a		 = static_cast<Type&&>(b);
		b		 = static_cast<Type&&>(tmp);

		return;
	}

	template <typename Type1, typename Type2>
		requires((_STD convertible_to<Type1 &&, Type2 &&>) && (_STD convertible_to<Type2 &&, Type1 &&>))
	constexpr void operator()(Type1&& a, Type2&& b) const
		noexcept((_STD is_nothrow_constructible_v<Type1, Type2>)&&(_STD is_nothrow_constructible_v<Type2, Type1>)&&(
			_STD is_nothrow_assignable_v<Type1, Type2>)&&(_STD is_nothrow_assignable_v<Type2, Type1>))
	{
		(*this)<_STD common_type_t<Type1, Type2>>(a, b);
	}

	template <typename Type1, typename Type2, size_t size>
	constexpr void operator()(Type1 (&t1)[size], Type2 (&t2)[size]) const noexcept(noexcept((*this)(t1[0], t2[0])))
		requires(requires() { (*this)(t1[0], t2[0]); })
	{
		for (size_t i = 0; i < size; ++i)
		{
			(*this)(t1[i], t2[i]);
		}

		return;
	}
};

constexpr inline __Swap_function swap { __Not_quite_object::__Construct_tag {} };

// 交换 a 与 b 的值。不可指定谓词 pred 与投影 proj
struct __Iter_swap_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	template <typename Iterator>
	constexpr void operator()(Iterator a, Iterator b) const noexcept(noexcept(swap(*a, *b)))
	{
		swap(*a, *b);

		return;
	}

	template <typename Iterator1, typename Iterator2>
	constexpr void operator()(Iterator1 a, Iterator2 b) const
	{
		Iterator1 tmp = _STD move(a);
		*a			  = _STD			move(_RANGES iter_move(b));
		*b			  = _STD			move(_RANGES iter_move(tmp));

		return;
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
		auto [end_for_range1, end_for_range2] = __default_swap_ranges(_unwrap_iterator<Sentinel1>(_STD move(first1)),
																	  _unwrap_sentinel<Iterator1>(_STD move(last1)),
																	  _unwrap_iterator<Sentinel2>(_STD move(first2)),
																	  _unwrap_sentinel<Iterator2>(_STD move(last2)));

		_seek_wrapped(first1, _STD move(end_for_range1));
		_seek_wrapped(first2, _STD move(end_for_range2));

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

		auto [end_for_range1, end_for_range2] = __default_swap_ranges(_unwrap_range_iterator<Range1>(_STD move(first1)),
																	  __uend(rng1),
																	  _unwrap_range_iterator<Range2>(_STD move(first2)),
																	  __uend(rng2));

		_seek_wrapped(first1, _STD move(end_for_range1));
		_seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2) };
	}
};

constexpr inline __Swap_ranges_function swap_ranges { __Not_quite_object::__Construct_tag {} };

// 将 [first, last) 范围内的元素依次调用 pred 。必须指定谓词 pred ，可指定投影 proj
struct __For_each_function: private __Not_quite_object
{
private:

	template <typename Iterator, typename Predicate>
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
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));

		auto [end_for_range, upred] =
			__default_for_each(_STD move(ufirst), _STD move(ulast), _STD move(pred), _check_function(proj));

		_seek_wrapped(first, _STD move(end_for_range));

		return { _STD move(first), _STD move(upred) };
	}

	/* function for_each() for 容器 强化版 */
	template <_RANGES input_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirectly_unary_invocable<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
					 _STD invoke(pred, _STD invoke(proj, _RANGES iter_move(__ubegin(rng))));
				 })
	inline for_each_result<_RANGES borrowed_iterator_t<Range>, Predicate>
		operator()(Range&& rng, Predicate pred, Projection proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto [end_for_range, upred] = __default_for_each(_unwrap_range_iterator<Range>(_STD move(first)),
														 __uend(rng),
														 _STD move(pred),
														 _check_function(proj));

		_seek_wrapped(first, _STD move(end_for_range));

		return { _STD move(first), _STD move(upred) };
	}
};

constexpr inline __For_each_function for_each { __Not_quite_object::__Construct_tag {} };

// 将 [first, first + n) 范围内的元素依次调用 pred 。必须指定谓词 pred ，可指定投影 proj
struct __For_each_n_function: private __Not_quite_object
{
private:

	template <typename Iterator, typename Predicate>
	using for_each_n_result = _RANGES in_fun_result<Iterator, Predicate>;

	// 统一调用方式
	template <typename Iterator, typename Predicate, typename Projection>
	static constexpr for_each_n_result<Iterator, Predicate> __default_for_each_n(Iterator first,
																				 _STD iter_difference_t<Iterator> n,
																				 Predicate						  pred,
																				 Projection proj) noexcept
	{
		for (_STD iter_difference_t<Iterator> i = 0; i < n; ++i, ++first)
		{
			_STD invoke(pred, _STD invoke(proj, *first));
		}

		return { _STD move(first), _STD move(pred) };
	}

public:

	using __Not_quite_object::__Construct_tag;

	/* function for_each_n() 标准版 */
	template <_STD input_iterator Iterator, typename Predicate, typename Projection = _STD identity>
		requires(requires(Iterator iter, Predicate pred, Projection proj) {
					 _STD invoke(pred, _STD invoke(proj, _RANGES iter_move(iter)));
				 })
	constexpr for_each_n_result<Iterator, Predicate> operator()(Iterator first,
																_STD iter_difference_t<Iterator> n,
																Predicate						 pred,
																Projection proj = {}) const noexcept
	{
		auto ufirst = _unwrap_iterator<Iterator>(_STD move(first));

		auto [end_of_range, upred] =
			__default_for_each_n(_STD move(ufirst), _STD move(n), _STD move(pred), _check_function(proj));

		_seek_wrapped(first, _STD move(end_of_range));

		return { _STD move(first), _STD move(upred) };
	}

	/* function for_each_n() for 容器 强化版 */
	template <_RANGES input_range Range, typename Predicate, typename Projection = _STD identity>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
					 _STD invoke(pred, _STD invoke(proj, _RANGES iter_move(__ubegin(rng))));
				 })
	constexpr for_each_n_result<_RANGES borrowed_iterator_t<Range>, Predicate>
		operator()(Range&& rng,
				   _STD iter_difference_t<_RANGES borrowed_iterator_t<Range>> n,
				   Predicate												  pred,
				   Projection												  proj = {}) const noexcept
	{
		auto first = _RANGES begin(rng);

		auto [end_for_range, upred] = __default_for_each_n(_unwrap_range_iterator<Range>(_STD move(first)),
														   _STD move(n),
														   _STD move(pred),
														   _check_function(proj));

		_seek_wrapped(first, _STD move(end_for_range));

		return { _STD move(first), _STD move(upred) };
	}
};

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
		auto ufirst1 = _unwrap_iterator<Sentinel1>(_STD move(first1));
		auto ulast1	 = _unwrap_sentinel<Iterator1>(_STD move(last1));

		auto ufirst2 = _unwrap_iterator<Sentinel2>(_STD move(first2));
		auto ulast2	 = _unwrap_sentinel<Iterator2>(_STD move(last2));

		return __default_equal(_STD move(ufirst1),
							   _STD move(ulast1),
							   _STD move(ufirst2),
							   _STD move(ulast2),
							   _check_function(pred),
							   _check_function(proj1),
							   _check_function(proj2));
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
		return __default_equal(__ubegin(rng1), __uend(rng1), __ubegin(rng2), __uend(rng2), pred, proj1, proj2);
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

		return;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	/* function fill() 标准版 */
	template <_STD forward_iterator Iterator, _STD sentinel_for<Iterator> Sentinel, typename Type>
	constexpr void operator()(Iterator first, Sentinel last, const Type& value) const noexcept
	{
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _unwrap_sentinel<Iterator>(_STD move(last));

		__default_fill(_STD move(ufirst), _STD move(ulast), _STD move(value));

		return;
	}

	/* function fill() for 容器 强化版 */
	template <_RANGES forward_range Range, typename Type>
	constexpr auto operator()(Range&& rng, const Type& value) const noexcept
	{
		__default_fill(__ubegin(rng), __uend(rng), _STD move(value));

		return;
	}
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
	}

	/* function fill_n() for 容器 强化版 */
	template <typename Type, _RANGES output_range<const Type&> Range>
	constexpr auto operator()(Range&& rng, _RANGES range_difference_t<Range> n, const Type& value) const noexcept
	{
		return __default_fill_n(__ubegin(rng), _STD move(n), _STD move(value));
	}
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
		auto check_pred = _check_function(pred);
		auto check_proj = _check_function(proj);

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
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))),
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))))
					 } noexcept -> _STD convertible_to<bool>;
				 })
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const _RANGES range_value_t<Range>&
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const
	{
		auto check_pred = _check_function(pred);
		auto check_proj = _check_function(proj);

		auto first = __ubegin(rng);
		auto last  = __uend(rng);
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
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))),
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))))
					 } noexcept -> _STD convertible_to<bool>;
				 })
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const Type&
		operator()(_STD initializer_list<Type> rng, Predicate pred = {}, Projection proj = {}) const
	{
		auto check_pred = _check_function(pred);
		auto check_proj = _check_function(proj);

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
		auto check_pred = _check_function(pred);
		auto check_proj = _check_function(proj);

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
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))),
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))))
					 } noexcept -> _STD convertible_to<bool>;
				 })
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const _RANGES range_value_t<Range>&
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_pred = _check_function(pred);
		auto check_proj = _check_function(proj);

		auto first = __ubegin(rng);
		auto last  = __uend(rng);
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
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))),
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))))
					 } noexcept -> _STD convertible_to<bool>;
				 })
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr const Type&
		operator()(_STD initializer_list<Type> rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto check_pred = _check_function(pred);
		auto check_proj = _check_function(proj);

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
		return _seek_wrapped(first,
							 __default_max_element(_unwrap_iterator<Sentinel>(_STD move(first)),
												   _unwrap_sentinel<Iterator, Sentinel>(_STD move(last)),
												   _check_function(pred),
												   _check_function(proj)));
	}

	/* function max_element() for 容器、仿函数 强化版 */
	template <
		_RANGES forward_range	   Range,
		typename Projection = _STD identity,
		_STD indirect_strict_weak_order<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate = _RANGES less>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
					 {
						 _STD invoke(pred,
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))),
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))))
					 } noexcept -> _STD convertible_to<bool>;
				 })
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_iterator_t<Range>
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return _rewrap_iterator(
			rng,
			__default_max_element(__ubegin(rng), __uend(rng), _check_function(pred), _check_function(proj)));
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
		return _seek_wrapped(first,
							 __default_min_element(_unwrap_iterator<Sentinel>(_STD move(first)),
												   _unwrap_sentinel<Iterator, Sentinel>(_STD move(last)),
												   _check_function(pred),
												   _check_function(proj)));
	}

	/* function min_element() for 容器、仿函数 强化版 */
	template <_RANGES forward_range		 Range,
			  typename Projection = _STD identity,
			  _STD indirect_strict_weak_order<_STD projected<_RANGES iterator_t<Range>, Projection>> Predicate =
				  _RANGES																			 greater>
		requires(requires(Range&& rng, Predicate pred, Projection proj) {
					 {
						 _STD invoke(pred,
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))),
									 _STD invoke(proj, _RANGES iter_move(__ubegin(rng))))
					 } noexcept -> _STD convertible_to<bool>;
				 })
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr auto
		operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		return _rewrap_iterator(
			rng,
			__default_min_element(__ubegin(rng), __uend(rng), _check_function(pred), _check_function(proj)));
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
			else					// 反之同理
			{
				*result = *first1;
				++first1;
			}

			++result;  // 每记录一个元素，result 前进 1 位
		}

		// 最后剩余元素拷贝到目的端。（两个序列一定有一个为空）
		if (first1 == last1)
		{
			auto [end_for_range1, end_for_range2] =
				_RANGES copy(_STD move(first2), _STD move(last2), _STD move(result));

			return { _STD move(first1), _STD move(end_for_range1), _STD move(end_for_range2) };
		}
		else  // first2 == last2
		{
			auto [end_for_range1, end_for_range2] =
				_RANGES copy(_STD move(first1), _STD move(last1), _STD move(result));

			return { _STD move(end_for_range1), _STD move(first2), _STD move(end_for_range2) };
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
		auto [end_for_range1, end_for_range2, end_for_range3] =
			__default_merge(_unwrap_iterator<Sentinel1>(_STD move(first1)),
							_unwrap_sentinel<Iterator1, Sentinel1>(_STD move(last1)),
							_unwrap_iterator<Sentinel2>(_STD move(first2)),
							_unwrap_sentinel<Iterator2, Sentinel2>(_STD move(last2)),
							_STD move(result),
							_check_function(pred),
							_check_function(proj1),
							_check_function(proj2));

		_seek_wrapped(first1, _STD move(end_for_range1));
		_seek_wrapped(first2, _STD move(end_for_range2));

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

		auto [end_for_range1, end_for_range2, end_for_range3] =
			__default_merge(_unwrap_range_iterator<Range1>(_STD move(first1)),
							__uend(rng1),
							_unwrap_range_iterator<Range2>(_STD move(first2)),
							__uend(rng2),
							_STD move(result),
							_check_function(pred),
							_check_function(proj1),
							_check_function(proj2));

		_seek_wrapped(first1, _STD move(end_for_range1));
		_seek_wrapped(first2, _STD move(end_for_range2));

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
		auto [end_for_range1, end_for_range2] = __default_transform(_unwrap_iterator<Sentinel>(_STD move(first)),
																	_unwrap_sentinel<Iterator>(_STD move(last)),
																	_STD move(result),
																	_check_function(pred),
																	_check_function(proj));

		_seek_wrapped(first, _STD move(end_for_range1));

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

		auto [end_for_range1, end_for_range2] = __default_transform(_unwrap_range_iterator<Range>(_STD move(first)),
																	__uend(rng),
																	_STD move(result),
																	_check_function(pred),
																	_check_function(proj));

		_seek_wrapped(first, _STD move(end_for_range1));

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
		auto [end_for_range1, end_for_range2, end_for_range3] =
			__default_transform(_unwrap_iterator<Sentinle1>(_STD move(first1)),
								_unwrap_sentinel<Iterator1>(_STD move(last1)),
								_unwrap_iterator<Sentinel2>(_STD move(first2)),
								_unwrap_sentinel<Iterator2>(_STD move(last2)),
								_STD move(result),
								_check_function(pred),
								_check_function(proj1),
								_check_function(proj2));

		_seek_wrapped(first1, _STD move(end_for_range1));
		_seek_wrapped(first2, _STD move(end_for_range2));

		return { _STD move(first1), _STD move(first2), _STD move(end_for_range3) };
	}

	// function transform() for 容器 强化版 2
	template <_RANGES input_range		  Range1,
			  _RANGES input_range		  Range2,
			  _STD weakly_incrementable	  OutIter,
			  _STD copy_constructible	  Predicate,
			  typename Projection1 = _STD identity,
			  typename Projection2 = _STD identity>
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

		auto [end_for_range1, end_for_range2, end_for_range3] =
			__default_transform(_unwrap_range_iterator<Range1>(_STD move(first1)),
								__uend(rng1),
								_unwrap_range_iterator<Range2>(_STD move(first2)),
								__uend(rng2),
								_STD move(result),
								_check_function(pred),
								_check_function(proj1),
								_check_function(proj2));

		_seek_wrapped(first1, _STD move(end_for_range1));
		_seek_wrapped(first2, _STD move(end_for_range2));

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

		return;
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
			__default_partial_sort(_unwrap_iterator<Sentinel>(_STD move(first)),
								   _unwrap_iterator<Sentinel>(_STD move(middle)),
								   _unwrap_sentinel<Iterator>(last),
								   _check_function(pred),
								   _check_function(proj));

			return last;
		}
		else
		{
			auto umiddle = _unwrap_iterator<Sentinel>(_STD move(middle));
			auto ulast	 = _get_last_iterator_unwrapped<Iterator>(umiddle, _STD move(last));

			_seek_wrapped(middle, ulast);

			__default_partial_sort(_unwrap_iterator<Sentinel>(_STD move(first)),
								   _STD move(umiddle),
								   _STD move(ulast),
								   _check_function(pred),
								   _check_function(proj));

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
			__default_partial_sort(__ubegin(rng),
								   _unwrap_range_iterator<Range>(_STD move(middle)),
								   __uend(rng),
								   _check_function(pred),
								   _check_function(proj));

			return _RANGES end(rng);
		}
		else
		{
			auto umiddle = _unwrap_range_iterator<Range>(_STD move(middle));
			auto ulast	 = _get_last_iterator_unwrapped(rng, umiddle);

			_seek_wrapped(middle, ulast);

			__default_partial_sort(__ubegin(rng),
								   _STD move(umiddle),
								   _STD move(ulast),
								   _check_function(pred),
								   _check_function(proj));

			return middle;
		}
	}

	// partial_sort() 的非检查版本
	template <typename Iterator, typename Sentinel, typename Predicate, typename Projection>
	constexpr void operator()(zh_No_inspection_required_function,
							  Iterator	 first,
							  Iterator	 middle,
							  Sentinel	 last,
							  Predicate	 pred,
							  Projection proj) const noexcept
	{
		__default_partial_sort(_STD move(first), _STD move(middle), _STD move(last), pred, proj);

		return;
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
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));

		ufirst =
			__default_insertion_sort(_STD move(ufirst), _STD move(ulast), _check_function(pred), _check_function(proj));

		_seek_wrapped(first, ufirst);

		return first;
	}

	// insertion_sort()	for 容器、仿函数 强化版
	template <_RANGES bidirectional_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto result = __default_insertion_sort(__ubegin(rng), __uend(rng), pred, proj);

		return _rewrap_iterator(rng, _STD move(result));
	}

	// insertion_sort() 的非检查版本
	template <typename Iterator, typename Sentinel, typename Predicate, typename Projection>
	constexpr Iterator operator()(zh_No_inspection_required_function,
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

		return;
	}

public:

	using __Not_quite_object::__Not_quite_object;

	template <_STD random_access_iterator Iterator, typename Predicate, typename Projection>
	constexpr void operator()(zh_No_inspection_required_function,
							  Iterator	 first,
							  Iterator	 middle,
							  Iterator	 last,
							  Predicate	 pred,
							  Projection proj) const noexcept
	{
		using diff_t = _STD iter_difference_t<Iterator>;

		const diff_t lenth = last - first;

		if ((_max_get_median_of_three_constant<diff_t>) < lenth)
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

		return;
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
	constexpr _RANGES subrange<Iterator> operator()(zh_No_inspection_required_function,
													Iterator   first,
													Sentinel   last,
													Predicate  pred,
													Projection proj) const noexcept
	{
		Iterator mid = first + ((last - first) >> 1);

		set_median_of_three(no_inspection_required, first, mid, _RANGES prev(last), pred, proj);

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
			auto [pfirst, plast] = partition(no_inspection_required, first, last, pred, proj);

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

		return;
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
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));

		_seek_wrapped(first, ufirst);

		const auto lenth = ulast - ufirst;

		__default_quick_sort(_STD move(ufirst),
							 _STD move(ulast),
							 _STD move(lenth),
							 _check_function(pred),
							 _check_function(proj));

		return first;
	}

	// quick_sort() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto	   ufirst = __ubegin(rng);
		auto	   ulast  = _get_last_iterator_unwrapped(rng);
		const auto lenth  = ulast - ufirst;

		__default_sort(_STD move(ufirst), ulast, lenth, pred, proj);

		return _rewrap_iterator(rng, _STD move(ufirst));
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

		return;
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
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));

		_seek_wrapped(first, ufirst);

		const auto lenth = ulast - ufirst;

		__default_merge_sort(_STD move(ufirst),
							 _STD move(ulast),
							 _STD move(lenth),
							 _check_function(pred),
							 _check_function(proj));

		return first;
	}

	// merge_sort() for 容器、仿函数 强化版
	template <_RANGES bidirectional_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto	   ufirst = __ubegin(rng);
		auto	   ulast  = _get_last_iterator_unwrapped(rng);
		const auto lenth  = ulast - ufirst;

		__default_merge_sort(_STD move(ufirst), ulast, lenth, pred, proj);

		return _rewrap_iterator(rng, _STD move(ufirst));
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
				< (_stl_threshold<_STD iter_difference_t<Iterator>>))  // 使用插入排序，如果元素数量足够少
			{
				insertion_sort(no_inspection_required, _STD move(first), _STD move(last), pred, proj);

				return;
			}

			if (ideal == 0)	 // 使用堆排序，如果递归深度足够深
			{
				partial_sort(no_inspection_required, _STD move(first), last, _STD move(last), pred, proj);

				return;
			}

			ideal = (ideal >> 1) + (ideal >> 2);  // allow 1.5 log2(N) divisions

			// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
			auto [pfirst, plast] = partition(no_inspection_required, first, last, pred, proj);

			// 递归调用，优先对较短的序列进行排序
			if ((pfirst - first) < (last - plast))
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
		auto ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto ulast	= _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));

		_seek_wrapped(first, ufirst);

		const auto lenth = ulast - ufirst;

		__default_sort(_STD move(ufirst),
					   _STD move(ulast),
					   _STD move(lenth),
					   _check_function(pred),
					   _check_function(proj));

		return first;
	}

	// sort() for 容器、仿函数 强化版
	template <_RANGES random_access_range Range, typename Predicate = _RANGES less, typename Projection = _STD identity>
		requires(__basic_concept_for_sort_function<_RANGES iterator_t<Range>, Predicate, Projection>)
	constexpr _RANGES borrowed_iterator_t<Range>
					  operator()(Range&& rng, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto	   ufirst = __ubegin(rng);
		auto	   ulast  = _get_last_iterator_unwrapped(rng);
		const auto lenth  = ulast - ufirst;

		__default_sort(_STD move(ufirst), ulast, lenth, pred, proj);

		return _rewrap_iterator(rng, _STD move(ufirst));
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
			auto [pfirst, plast] = partition(no_inspection_required, first, last, pred, proj);

			if (nth < pfirst)				// 如果 指定位置 < 右段起点，（即 nth 位于右段）
			{
				first = _STD move(pfirst);	// 对右段实施分割
			}
			else							// 否则（nth 位于左段）
			{
				last = _STD move(plast);	// 对左段实施分割
			}
		}

		insertion_sort(no_inspection_required, _STD move(first), _STD move(last), pred, proj);

		return;
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
		auto unth  = _unwrap_iterator<Sentinel>(_STD move(nth));
		auto ulast = _get_last_iterator_unwrapped<Iterator>(unth, _STD move(last));

		_seek_wrapped(nth, ulast);

		__default_nth_element(_STD move(_unwrap_iterator<Sentinel>(_STD move(first))),
							  _STD move(unth),
							  _STD move(ulast),
							  _check_function(pred),
							  _check_function(proj));

		return nth;
	}

	/* function nth_element() for 容器、仿函数 强化版 */
	template <_RANGES random_access_range  Range,
			  _STD random_access_iterator  Iterator,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
	constexpr auto operator()(Range&& rng, Iterator nth, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto unth  = _unwrap_range_iterator<Range>(_STD move(nth));
		auto ulast = [&]
		{
			if constexpr (_RANGES common_range<Range>)
			{
				return __uend(rng);
			}
			else if constexpr (_RANGES sized_range<Range>)
			{
				return _RANGES next(__ubegin(rng), _RANGES distance(rng));
			}
			else
			{
				return _RANGES next(unth, __uend(rng));
			}
		}();

		_seek_wrapped(nth, ulast);

		__default_nth_element(__ubegin(rng),
							  _STD move(unth),
							  _STD move(ulast),
							  _check_function(pred),
							  _check_function(proj));

		return nth;
	}
};

constexpr inline __Nth_element_function nth_element { __Not_quite_object::__Construct_tag {} };

/*------------------------------------------------------------------------------------------------*/



/* 此处实现一些关于二分查找的算法 */

template <typename Iterator, typename Type, typename Predicate, typename Projection>
concept __basic_concept_for_binary_search_function =
	requires(Iterator iter, const Type& value, Predicate pred, Projection proj) {
		{
			_STD invoke(pred, _STD invoke(proj, _RANGES iter_move(iter)), value)
		} noexcept -> _STD convertible_to<bool>;

		{
			_STD invoke(pred, value, _STD invoke(proj, _RANGES iter_move(iter)))
		} noexcept -> _STD convertible_to<bool>;

		{
			_STD invoke(pred, _RANGES iter_move(iter), _RANGES iter_move(iter))
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
			half = static_cast<diff_t>(lenth >> 1);

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
		auto	   ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		const auto lenth  = _RANGES distance(ufirst, _unwrap_sentinel<Iterator, Sentinel>(_STD move(last)));

		ufirst = __default_lower_bound(_STD move(ufirst),
									   _STD move(lenth),
									   _STD move(value),
									   _check_function(pred),
									   _check_function(proj));

		_seek_wrapped(first, _STD move(ufirst));

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
		auto result = __default_lower_bound(__ubegin(rng),
											_STD move(_RANGES distance(rng)),
											_STD move(value),
											_check_function(pred),
											_check_function(proj));

		return _rewrap_iterator(rng, _STD move(result));
	}

	// lower_bound() 的非检查版本
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr Iterator operator()(zh_No_inspection_required_function,
								  Iterator first,
								  _STD iter_difference_t<Iterator> lenth,
								  const Type&					   value,
								  Predicate						   pred,
								  Projection					   proj) const noexcept
	{
		return __default_lower_bound(_STD move(first), _STD move(lenth), _STD move(value), pred, proj);
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
			half = static_cast<diff_t>(lenth >> 1);

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
		auto	   ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		const auto lenth  = _RANGES distance(ufirst, _unwrap_sentinel<Iterator, Sentinel>(_STD move(last)));

		ufirst = __default_upper_bound(_STD move(ufirst),
									   _STD move(lenth),
									   _STD move(value),
									   _check_function(pred),
									   _check_function(proj));

		_seek_wrapped(first, _STD move(ufirst));

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
		auto result = __default_upper_bound(__ubegin(rng),
											_STD move(_RANGES distance(rng)),
											_STD move(value),
											_check_function(pred),
											_check_function(proj));

		return _rewrap_iterator(rng, _STD move(result));
	}

	// upper_bound() 的非检查版本
	template <typename Iterator, typename Type, typename Predicate, typename Projection>
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略")
	constexpr Iterator operator()(zh_No_inspection_required_function,
								  Iterator first,
								  _STD iter_difference_t<Iterator> lenth,
								  const Type&					   value,
								  Predicate						   pred,
								  Projection					   proj) const noexcept
	{
		return __default_upper_bound(_STD move(first), _STD move(lenth), _STD move(value), pred, proj);
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

		while (0 < lenth)											   // 如果整个区间尚未迭代完毕
		{
			half = static_cast<diff_t>(lenth >> 1);					   // 找出中间位置

			middle = _RANGES next(first, half);						   // 设定中央迭代器

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
			else						  // 如果 中央元素 == 指定值
			{
				// 在前半段寻找上限
				auto begin { lower_bound(no_inspection_required, _STD move(first), half, value, pred, proj) };

				// 在后半段寻找下限
				lenth = static_cast<diff_t>(half + 1);

				auto end { upper_bound(no_inspection_required, _STD move(++middle), lenth, value, pred, proj) };

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
		auto	   ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto	   ulast  = _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));
		const auto lenth  = ulast - ufirst;

		auto result = __default_equal_range(_STD move(ufirst),
											_STD move(lenth),
											_STD move(value),
											_check_function(pred),
											_check_function(proj));

		return _rewrap_subrange<_RANGES subrange<Iterator>>(first, _STD move(result));
	}

	/* function equal_range() for 容器、仿函数 强化版 */
	template <_RANGES forward_range Range,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<_RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr _RANGES borrowed_subrange_t<Range>
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto first		 = _RANGES begin(rng);
		const auto lenth = _RANGES distance(rng);

		auto result = __default_equal_range(_STD move(__ubegin(rng)),
											_STD move(lenth),
											_STD move(value),
											_check_function(pred),
											_check_function(proj));

		return _rewrap_subrange<_RANGES borrowed_subrange_t<Range>>(first, _STD move(result));
	}
};

constexpr inline __Equal_range_function equal_range { __Not_quite_object::__Construct_tag {} };

// 返回一个布尔值，表示有序序列中是否存在与给定值相同的元素。可指定谓词 pred 与投影 proj
struct __Binary_search_function: private __Not_quite_object
{
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
		auto	   ufirst = _unwrap_iterator<Sentinel>(_STD move(first));
		auto	   ulast  = _get_last_iterator_unwrapped<Iterator>(ufirst, _STD move(last));
		const auto lenth  = ulast - ufirst;

		ufirst = lower_bound(no_inspection_required,
							 _STD move(ufirst),
							 _STD move(lenth),
							 _STD move(value),
							 _check_function(pred),
							 _check_function(proj));

		return (ufirst != ulast) && (!(_STD invoke(pred, value, _STD invoke(proj, *ufirst))));
	}

	/* function binary_search() for 容器、仿函数 强化版 */
	template <_RANGES forward_range Range,
			  typename Type,
			  typename Predicate  = _RANGES less,
			  typename Projection = _STD   identity>
		requires(__basic_concept_for_binary_search_function<_RANGES iterator_t<Range>, Type, Predicate, Projection>)
	_NODISCARD_MSG(L"此函数的返回值不应该被忽略") constexpr bool
		operator()(Range&& rng, const Type& value, Predicate pred = {}, Projection proj = {}) const noexcept
	{
		auto	   ufirst = __ubegin(rng);
		const auto lenth  = _RANGES distance(rng);

		ufirst = lower_bound(no_inspection_required,
							 _STD move(ufirst),
							 _STD move(lenth),
							 _STD move(value),
							 _check_function(pred),
							 _check_function(proj));

		return (ufirst != __uend(rng)) && (!(_STD invoke(pred, value, _STD invoke(proj, *ufirst))));
	}
};

constexpr inline __Binary_search_function binary_search { __Not_quite_object::__Construct_tag {} };

_END_NAMESPACE_ZHANG

#endif	// _HAS_CXX20
