#pragma once

#include <string_view>
#include <type_traits>

#include <algorithm>
#include <climits>
#include <compare>
#include <concepts>
#include <format>
#include <functional>
#include <iostream>
#include <iterator> // for MSVC STL _STD xxx_iterator_tag
#include <memory>
#include <new>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include <version>



#ifndef _STD
	#define _STD ::std::
#endif // !_STD

#ifndef _NODISCARD
	#define _NODISCARD [[nodiscard]]
#endif // !_NODISCARD

#ifndef _NORETURN
	#define _NORETURN [[noreturn]]
#endif // !_NORETURN

#ifndef __move
	#define __move(cont) ::std::move(cont)
#endif // !__move

#ifndef __invoke
	#define __invoke(...) std::invoke(__VA_ARGS__)
#endif // !__invoke


#define __cove_type(cont, type)			static_cast<type>(cont)
#define __init_type(initCont, initType) __cove_type(initCont, initType)



#if !(__cplusplus < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 __cplusplus
	#endif // !__HAS_CPP20

#elif !(__cplusplus < 201703L)

	#ifndef __HAS_CPP17
		#define __HAS_CPP17 __cplusplus
	#endif // !__HAS_CPP17

#endif

#if !(_MSVC_LANG < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 _MSVC_LANG
	#endif // !__HAS_CPP20

#elif !(_MSVC_LANG < 201703L)

	#ifndef __HAS_CPP17
		#define __HAS_CPP17 _MSVC_LANG
	#endif // !__HAS_CPP17

#endif


#if __HAS_CPP20

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

	#ifndef _RANGES
		#define _RANGES ::std::ranges::
	#endif // !_RANGES

#endif	   // __HAS_CPP20



#if __HAS_CPP20

	#ifndef __invoke
		#define __invoke(...) std::invoke(__VA_ARGS__)
	#endif // !__invoke

#endif	   // __HAS_CPP20


#if __HAS_CPP20

// 以下是 基础型别 的要求
template <typename T>
concept __is_c_array = _STD is_array_v<T>;

template <typename T>
concept __is_c_pointer = _STD is_pointer_v<T>;

template <typename T>
concept __is_basic_compound = !(_STD is_compound_v<T>);

template <typename T>
concept __not_basic_compound = _STD is_compound_v<T>;


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


// 以下是 哨兵 的要求
//template <__is_input_iterator Iterator>
//using __sentinel_for_iterator = _STD sentinel_for<Iterator>;

// 针对 msvc 的检查函数
	#ifdef _MSC_VER
template <class Function>
constexpr decltype(auto) __global_check_fun(Function& fun) noexcept
{
	return _STD _Pass_fn(fun);
}
	#endif

// 针对 clang 的检查函数
	#ifdef __MINGW32__
template <typename Function>
constexor decltype(auto) __global_check_fun(Function& fun)
{
	return __gnu_cxx::__ops::__iter_comp_iter(fun);
}
	#endif

// 针对 mingw 的检查函数
	#ifdef __GNUC__
template <typename Function>
constexor decltype(auto) __global_check_fun(Function& fun)
{
	return fun;
}
	#endif

#endif // __HAS_CPP20


// 针对 自定义打印函数 的
#ifdef __HAS_CPP20

	#if __has_include(<format>)&&!(defined __CPP20_PRINT)
		#define __CPP20_PRINT
template <typename T>
concept __basic_msg_type = (__is_basic_compound<T>) || requires(T msg) { requires(noexcept(_STD string(msg))); } ||
						   requires(T msg) { requires(noexcept(_STD string_view(msg))); };
	#endif

#endif // __HAS_CPP20


// 以下是一些 SGI STL 预定义全局变量 和 宏
// 1、
constexpr inline auto __stl_threshold = 16;

// 2、
#ifndef __STL_TEMPLATE_NULL
	#define __STL_TEMPLATE_MULL template <>
#endif // !__STL_TEMPLATE_NULL

// 以下是一些自定义的命名空间
// 1、
#ifndef __BEGIN_NP_ZHANG
	#define __BEGIN_NP_ZHANG   \
		inline namespace zhang \
		{
#endif // !__BEGIN_NP_ZHANG

#ifndef __END_NP_ZHANG
	#define __END_NP_ZHANG }
#endif // !__END_NP_ZHANG

// 2、
#ifndef __BEFIN_NEW_NP
	#define __BEGIN_NEW_NP(name) \
		inline namespace name    \
		{
#endif // !__BEFIN_NEW_NP

#ifndef __END_NEW_NP
	#define __END_NEW_NP(name) }
#endif // !__END_NEW_NP


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
