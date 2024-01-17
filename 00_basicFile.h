#pragma once

#include <string_view>
#include <type_traits>

#include <algorithm>
#include <climits>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>
#include <version>



#if !(__cplusplus < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 __cplusplus
	#endif // !__HAS_CPP20

#elif !(_MSVC_LANG < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 _MSVC_LANG
	#endif // !__HAS_CPP20

#endif



#if __HAS_CPP20

	#include <format>
	#include <ranges>
	#include <string_view>
	#include <compare>
	#include <concepts>


	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#ifndef _RANGES
		#define _RANGES ::std::ranges::
	#endif // !_RANGES

	#ifndef _NODISCARD
		#define _NODISCARD [[nodiscard]]
	#endif // !_NODISCARD

	#ifndef _NORETURN
		#define _NORETURN [[noreturn]]
	#endif // !_NORETURN

	#ifndef __cove_type
		#define __cove_type(cont, type) static_cast<type>(cont)
	#endif // !__cove_type

	#ifndef __init_type
		#define __init_type(initCont, initType) __cove_type(initCont, initType)
	#endif // !__init_type

	#ifndef __move
		#define __move(cont) ::std::move(cont)
	#endif // !__move

	#ifndef __invoke
		#define __invoke(...) std::invoke(__VA_ARGS__)
	#endif // !__invoke

	#ifndef __begin_for_range
		#define __begin_for_range(cont) ::std::ranges::begin(cont)
	#endif // !__begin_for_range

	#ifndef __end_for_range
		#define __end_for_range(cont) ::std::ranges::end(cont)
	#endif // !__end_for_range

	#ifndef __begin_for_range_with_move
		#define __begin_for_range_with_move(cont) std::move(::std::ranges::begin(cont))
	#endif // !__begin_for_range

	#ifndef __end_for_range_with_move
		#define __end_for_range_with_move(cont) std::move(::std::ranges::end(cont))
	#endif // !__end_for_range

	#ifndef __STL_TEMPLATE_NULL
		#define __STL_TEMPLATE_MULL template <>
	#endif // !__STL_TEMPLATE_NULL

	#ifndef __BEGIN_NAMESPACE_ZHANG
		#define __BEGIN_NAMESPACE_ZHANG \
			namespace zhang             \
			{
	#endif // !__BEGIN_NAMESPACE_ZHANG

	#ifndef __END_NAMESPACE_ZHANG
		#define __END_NAMESPACE_ZHANG }
	#endif // !__END_NAMESPACE_ZHANG

	#ifndef __BEGIN_NEW_NAMESPACE
		#define __BEGIN_NEW_NAMESPACE(name) \
			namespace name                  \
			{
	#endif // !__BEGIN_NEW_NAMESPACE

	#ifndef __END_NEW_NAMESPACE
		#define __END_NEW_NAMESPACE(name) }
	#endif // !__END_NEW_NAMESPACE

	#ifndef __BEGIN_INLINE_NAMESPACE
		#define __BEGIN_INLINE_NAMESPACE(name) \
			inline namespace name              \
			{
	#endif // !__BEGIN_INLINE_NAMESPACE

	#ifndef __END_INLINE_NAMESPACE
		#define __END_INLINE_NAMESPACE(name) }
	#endif // !__END_INLINE_NAMESPACE

	#ifndef __stl_threshold
constexpr inline auto __stl_threshold = 16;
	#endif // !__stl_threshold



/*-----------------------------------------------------------------------------------------------------*/



// 以下是 基础型别 的要求
template <typename T>
concept __is_c_array = _STD is_array_v<T>;

template <typename T>
concept __is_compound_type = _STD is_compound_v<T>;

template <typename T>
concept __not_compound_type = (!__is_compound_type<T>);


// 以下是 容器型别 的基础要求
template <typename T>
concept __is_range = _RANGES range<T>;

template <typename T>
concept __is_random_access_range = _RANGES random_access_range<T>;

template <typename T>
concept __is_bidirectional_range = _RANGES bidirectional_range<T>;

template <typename T>
concept __is_forward_range = _RANGES forward_range<T>;

template <typename T>
concept __is_output_range = _RANGES output_range<T, _RANGES range_value_t<T>>;

template <typename T>
concept __is_input_range = _RANGES input_range<T>;


// 从 基础迭代器 或 容器 获取信息
template <_STD input_or_output_iterator Iterator>
using __difference_type_for_iter = typename _STD iter_difference_t<Iterator>;

template <_STD input_or_output_iterator Iterator>
using __value_type_for_iter = typename _STD iter_value_t<Iterator>;

template <_RANGES range Range>
using __difference_type_for_range = typename _RANGES range_difference_t<Range>;

template <_RANGES range Range>
using __value_type_for_range = typename _RANGES range_value_t<Range>;

template <_RANGES range Range>
using __iterator_type_for_range = typename _RANGES iterator_t<Range>;

template <_RANGES range Range>
using __sentinel_type_for_range = typename _RANGES sentinel_t<Range>;

template <_STD input_or_output_iterator Iterator>
using __type_tag_for_iter = typename _STD iterator_traits<Iterator>::iterator_category;


// 以下是 迭代器型别 的定义
template <typename T>
concept __is_input_iterator = _STD input_iterator<T>;

template <typename T>
concept __is_output_iterator = _STD output_iterator<T, __value_type_for_iter<T>>;

template <typename T>
concept __is_bidirectional_iterator = _STD bidirectional_iterator<T>;

template <typename T>
concept __is_forward_iterator = _STD forward_iterator<T>;

template <typename T>
concept __is_random_access_iterator = _STD random_access_iterator<T>;

template <typename T> // 为了使 C风格指针 和 C风格数组 被正确识别为迭代器的 特别定义
concept __is_iter_or_array = (__is_input_iterator<T>) || (_STD is_array_v<T>);

// Iterator 没有 _Unwrapped()
template <typename Iterator, typename = void>
constexpr inline bool __has_nothrow_unwrapped = false;

// Iterator 或有 _Unwrapped()
template <typename Iterator>
constexpr inline bool __has_nothrow_unwrapped<Iterator, _STD void_t<decltype(_STD declval<Iterator>()._Unwrapped())>> =
	noexcept(_STD declval<Iterator>()._Unwrapped());



/*-----------------------------------------------------------------------------------------------------*/


// Iterator 不允许继承展开
template <typename Iterator, typename = void>
constexpr inline bool __is_allow_inheriting_unwrap = true;

// Iterator 或允许继承展开
template <typename Iterator>
constexpr inline bool
	__is_allow_inheriting_unwrap<Iterator, _STD void_t<typename Iterator::_Prevent_inheriting_unwrap>> =
		(_STD is_same_v<Iterator, typename Iterator::_Prevent_inheriting_unwrap>);

// Wrapped 是弱展开
template <typename Wrapped>
concept __is_weakly_unwrappable = (__is_allow_inheriting_unwrap<_STD remove_cvref_t<Wrapped>>)&&(
	requires(Wrapped&& wrap) { _STD forward<Wrapped>(wrap)._Unwrapped(); });

// Sentinel 是弱展开哨兵
template <typename Sentinel>
concept __is_weakly_unwrappable_sentinel = __is_weakly_unwrappable<const _STD remove_reference_t<Sentinel>&>;

// Iterator 是弱展开迭代器
template <typename Iterator>
concept __is_weakly_unwrappable_iterator =
	(__is_weakly_unwrappable<Iterator>)&&(requires(Iterator&& iterator, _STD remove_cvref_t<Iterator>& mut_iter) {
		mut_iter._Seek_to(_STD forward<Iterator>(iterator)._Unwrapped());
	});

// 展开迭代器类型
template <typename Iterator, typename Sentinel>
using __unwrap_iterator_type = _STD remove_cvref_t<decltype(__unwrap_iterator<Sentinel>(_STD declval<Iterator>()))>;

// 展开哨兵类型
template <typename Sentinel, typename Iterator>
using __unwrap_sentinel_type = _STD remove_cvref_t<decltype(__unwrap_iterator<Iterator>(_STD declval<Sentinel>()))>;

// 判断弱展开哨兵
template <typename Sentinel, typename Iterator>
concept __is_unwrappable_sentinel_for =
	(__is_weakly_unwrappable_sentinel<Sentinel>)&&(__is_weakly_unwrappable_iterator<Iterator>)&&(
		requires(Iterator&& iterator, const _STD remove_reference_t<Sentinel>& sentinel) {
			{
				sentinel._Unwrapped()
			} -> _STD sentinel_for<decltype(_STD forward<Iterator>(iterator)._Unwrapped())>;
		});

/*-----------------------------------------------------------------------------------------------------*/

// 展开头迭代器
template <typename Sentinel, typename Iterator>
_NODISCARD constexpr decltype(auto)
	__unwrap_iterator(Iterator&& iterator) noexcept((!__is_unwrappable_sentinel_for<Sentinel, Iterator>) ||
													(__has_nothrow_unwrapped<Iterator>))
{
	_STL_INTERNAL_STATIC_ASSERT(_STD sentinel_for<_STD remove_cvref_t<Sentinel>, _STD remove_cvref_t<Iterator>>);

	if constexpr (_STD is_pointer_v<_STD remove_cvref_t<Iterator>>)
	{
		return iterator + 0;
	}
	else if constexpr (__is_unwrappable_sentinel_for<Sentinel, Iterator>)
	{
		return static_cast<Iterator&&>(iterator)._Unwrapped();
	}
	else
	{
		return static_cast<Iterator&&>(iterator);
	}
}

// 展开哨兵
template <typename Iterator, typename Sentinel>
_NODISCARD constexpr decltype(auto)
	__unwrap_sentinel(Sentinel&& sentinel) noexcept((!__is_unwrappable_sentinel_for<Sentinel, Iterator>) ||
													(__has_nothrow_unwrapped<Sentinel>))
{
	_STL_INTERNAL_STATIC_ASSERT(_STD sentinel_for<_STD remove_cvref_t<Sentinel>, _STD remove_cvref_t<Iterator>>);

	if constexpr (_STD is_pointer_v<_STD remove_cvref_t<Sentinel>>) // 如果是指针，直接返回
	{
		return sentinel + 0;
	}
	else if constexpr (__is_unwrappable_sentinel_for<Sentinel, Iterator>) // 如果是弱展开哨兵，返回 _Unwrapped()
	{
		return static_cast<Sentinel&&>(sentinel)._Unwrapped();
	}
	else // 否则，返回 Sentinel&&
	{
		return static_cast<Sentinel&&>(sentinel);
	}
}

// 获取尾迭代器
template <__is_forward_iterator Iterator, typename Sentinel>
	requires(_STD sentinel_for<_STD remove_cvref_t<Sentinel>, Iterator>)
_NODISCARD constexpr __unwrap_iterator_type<Iterator, Sentinel>
	__get_last_iterator_unwrapped(const __unwrap_iterator_type<Iterator, Sentinel>& first, Sentinel&& last)
{
	if constexpr (_STD
					  is_same_v<__unwrap_iterator_type<Iterator, Sentinel>, __unwrap_sentinel_type<Sentinel, Iterator>>)
	{
		return __unwrap_sentinel<Iterator>(_STD forward<Sentinel>(last));
	}
	else
	{
		return _RANGES next(first, __unwrap_sentinel<Iterator>(_STD forward<Sentinel>(last)));
	}
}

/*-----------------------------------------------------------------------------------------------------*/


template <typename Function>
struct __get_ref_function
{
	// 按值传递函数对象作为引用
	Function& fun;

	template <class... Args>
	constexpr decltype(auto) operator()(Args&&... args)
	{
		// forward function call operator
		if constexpr (_STD is_member_pointer_v<Function>)
		{
			return __invoke(fun, _STD forward<Args>(args)...);
		}
		else
		{
			return fun(_STD forward<Args>(args)...);
		}
	}
};

template <typename Function>
_NODISCARD constexpr auto __check_function(Function& fun) noexcept
{
	constexpr bool __pass_by_value = _STD conjunction_v<_STD bool_constant<sizeof(Function) <= sizeof(void*)>,
														_STD is_trivially_copy_constructible<Function>,
														_STD is_trivially_destructible<Function>>;
	if constexpr (__pass_by_value)
	{
		return fun;
	}
	else
	{
		return __get_ref_function<Function> { fun }; // 通过“引用”传递函子
	}
}

class __not_quite_object
{
public:

	/*
	 * 库中的一些重载集具有这样的特性，即它们的组成函数模板对参数相关名称查找（ADL）不可见，
	 * 并且当通过非限定名称查找找到 ADL 时，它们会禁止 ADL 。此属性允许将这些重载集实现为函数对象。
	 * 我们从这种类型派生出这样的函数对象，以删除一些典型的对象行为，这有助于用户避免依赖于他们未指定的对象性。
	*/

	struct __construct_tag
	{
		explicit __construct_tag() = default;
	};

	__not_quite_object() = delete;

	constexpr explicit __not_quite_object(__construct_tag) noexcept
	{
	}

	__not_quite_object(const __not_quite_object&)			 = delete;
	__not_quite_object& operator=(const __not_quite_object&) = delete;

	void operator&() const = delete;

protected:

	~__not_quite_object() = default;
};

#endif // __HAS_CPP20
