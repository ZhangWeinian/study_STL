#pragma once

#include <string_view>
#include <type_traits>

#include <algorithm>
#include <climits>
#include <compare>
#include <concepts>
#include <fcntl.h>
#include <functional>
#include <io.h>
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

#elif !(__cplusplus < 201703L)

	#ifndef __HAS_CPP17
		#define __HAS_CPP17 __cplusplus
	#endif // !__HAS_CPP17

#endif

#if !(_MSVC_LANG < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 _MSVC_LANG
		#define __HAS_MSVC
	#endif // !__HAS_CPP20

#elif !(_MSVC_LANG < 201703L)

	#ifndef __HAS_CPP17
		#define __HAS_CPP17 _MSVC_LANG
		#define __HAS_MSVC
	#endif // !__HAS_CPP17

#endif



#if __HAS_CPP20

	#if __has_include(<format>)
	#else
		#include <format>
	#endif // __has_include(<format>)

	#if __has_include(<ranges>)
	#else
		#include <ranges>
	#endif // __has_include(<ranges>)

	#if __has_include(<string_view>)
	#else
		#include <string_view>
	#endif // __has_include(<string_view>)


	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#ifndef _NODISCARD
		#define _NODISCARD [[nodiscard]]
	#endif // !_NODISCARD

	#ifndef _NORETURN
		#define _NORETURN [[noreturn]]
	#endif // !_NORETURN

	#ifndef _RANGES
		#define _RANGES ::std::ranges::
	#endif // !_RANGES



	#define __cove_type(cont, type)			static_cast<type>(cont)
	#define __init_type(initCont, initType) __cove_type(initCont, initType)



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

// 为了使 C风格指针 和 C风格数组 被正确识别为迭代器的 特别定义
template <typename T>
concept __is_iter_or_array = (__is_input_iterator<T>) || (_STD is_array_v<T>);



// 针对 format() 格式的一般泛化 的参数要求
template <typename T>
concept __basic_msg_type = (__not_compound_type<T>) || (requires(T msg) { noexcept(_STD string(msg)); }) ||
						   (requires(T msg) { noexcept(_STD wstring(msg)); });



// 针对 msvc 的检查函数
	#ifdef __HAS_MSVC
template <class Function>
constexpr decltype(auto) __global_check_fun(Function& fun) noexcept
{
	return _STD _Pass_fn(_STD forward<Function&>(fun));
}

		#define __has_check_fun

	#endif

// 针对 clang 的检查函数
	#ifdef __MINGW32__
template <typename Function>
constexor decltype(auto) __global_check_fun(Function& fun) noexcept
{
	return __gnu_cxx::__ops::__iter_comp_iter(_STD forward<Function&>(fun));
}

		#define __has_check_fun

	#endif

// 针对 mingw 的检查函数
	#ifdef __GNUC__
template <typename Function>
constexor decltype(auto) __global_check_fun(Function& fun) noexcept
{
	return fun;
}

		#define __has_check_fun

	#endif

	#ifdef __has_check_fun
	#else

template <class Function>
constexpr decltype(auto) __global_check_fun(Function& fun) noexcept
{
	return fun;
}

	#endif // __has_check_fun


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
#ifndef __BEGIN_NAMESPACE_ZHANG
	#define __BEGIN_NAMESPACE_ZHANG \
		namespace zhang             \
		{
#endif // !__BEGIN_NAMESPACE_ZHANG

#ifndef __END_NAMESPACE_ZHANG
	#define __END_NAMESPACE_ZHANG }
#endif // !__END_NAMESPACE_ZHANG

// 2、
#ifndef __BEFIN_NEW_NAMESPACE
	#define __BEGIN_NEW_NAMESPACE(name) \
		namespace name                  \
		{
#endif // !__BEFIN_NEW_NAMESPACE

#ifndef __END_NEW_NAMESPACE
	#define __END_NEW_NAMESPACE(name) }
#endif // !__END_NEW_NAMESPACE

// 3、基础类
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
