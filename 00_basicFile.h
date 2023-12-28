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
	#include <ranges>
#endif // __HAS_CPP20


#if __HAS_CPP20

	#ifndef __begin_for_container
		#define __begin_for_container(cont) ::std::ranges::begin(cont)
	#endif // !__begin_for_container

	#ifndef __end_for_container
		#define __end_for_container(cont) ::std::ranges::end(cont)
	#endif // !__end_for_container

	#ifndef __begin_for_container_move
		#define __begin_for_container_move(cont) std::move(::std::ranges::begin(cont))
	#endif // !__begin_for_container

	#ifndef __end_for_container_move
		#define __end_for_container_move(cont) std::move(::std::ranges::end(cont))
	#endif // !__end_for_container

	#ifndef _RANGES
		#define _RANGES ::std::ranges::
	#endif // !_RANGES

#else

	#ifndef __begin_for_container
		#define __begin_for_container(cont) ::std::begin(cont)
	#endif // !__begin_for_container

	#ifndef __end_for_container
		#define __end_for_container(cont) ::std::end(cont)
	#endif // !__end_for_container

	#ifndef __begin_for_container_move
		#define __begin_for_container_move(cont) std::move(::std::begin(cont))
	#endif // !__begin_for_container

	#ifndef __end_for_container_move
		#define __end_for_container_move(cont) std::move(::std::end(cont))
	#endif // !__end_for_container

	#ifndef _RANGES
		#define _RANGES ::std::ranges::
	#endif // !_RANGES

#endif	   // __HAS_CPP20



#if __HAS_CPP20

// 以下是 基础型别 的定义
template <typename T>
concept __is_c_array = _STD is_array_v<T>;

template <typename T>
concept __is_c_pointer = _STD is_pointer_v<T>;

template <typename T>
concept __is_basic_compound = !(_STD is_compound_v<T>);

template <typename T>
concept __is_range = _RANGES range<T>;



// 以下是 容器型别 的定义
template <typename T>
concept __is_container_without_c_array = requires(T p) {
	typename T::value_type;
	typename T::iterator;
	requires(__is_range<T>);
};

template <typename T>
concept __is_container_or_c_array = __is_container_without_c_array<T> || __is_c_array<T>;



// 从 基础迭代器 或 容器 获取信息
template <_STD input_or_output_iterator Iterator>
using __difference_type_for_iter = typename _STD iterator_traits<Iterator>::difference_type;

template <_STD input_or_output_iterator Iterator>
using __value_type_for_iter =
	typename _STD iter_value_t<Iterator>; // typename _STD iterator_traits<Iterator>::value_type;

template <__is_container_without_c_array Container>
using __difference_type_for_con = typename Container::difference_type;

template <__is_container_without_c_array Container>
using __value_type_for_con = typename Container::value_type;

template <_STD input_or_output_iterator Iterator>
using __get_iter_type_tag = typename _STD iterator_traits<Iterator>::iterator_category;



// 以下是 迭代器型别 的定义
template <typename T>
concept __is_input_iterator = _STD input_iterator<T>;

template <typename T>
concept __is_input_iterator_without_c_pointer = !(__is_c_pointer<T>)&&(__is_input_iterator<T>);

template <typename T>
concept __is_output_iterator = _STD output_iterator<T, __value_type_for_iter<T>>;

template <typename T>
concept __is_output_iterator_without_c_pointer = !(__is_c_pointer<T>)&&(__is_output_iterator<T>);

template <typename T>
concept __is_bidirectional_iterator = _STD bidirectional_iterator<T>;

template <typename T>
concept __is_bidirectional_iterator_without_c_pointer = !(__is_c_pointer<T>)&&(__is_bidirectional_iterator<T>);

template <typename T>
concept __is_forward_iterator = _STD forward_iterator<T>;

template <typename T>
concept __is_forward_iterator_without_c_pointer = !(__is_c_pointer<T>)&&(__is_forward_iterator<T>);

template <typename T>
concept __is_random_access_iterator = _STD random_access_iterator<T>;

template <typename T>
concept __is_random_access_iterator_without_c_pointer = !(__is_c_pointer<T>)&&(__is_random_access_iterator<T>);


// 针对 msvc 的检查函数
	#ifdef _MSC_VER
template <class Function>
constexpr decltype(auto) __check_fun(Function& fun) noexcept
{
	return _STD _Pass_fn(fun);
}
	#endif

// 针对 clang 的检查函数
	#ifdef __MINGW32__
template <typename Function>
constexor decltype(auto) __check_fun(Function& fun)
{
	return __gnu_cxx::__ops::__iter_comp_iter(fun);
}
	#endif

// 针对 mingw 的检查函数
	#ifdef __GNUC__
template <typename Function>
constexor decltype(auto) __check_fun(Function& fun)
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


// 以下全部都是 SGI STL 预定义全局变量
// 1、
inline constexpr auto __stl_threshold = 16;

// 2、
#ifndef __STL_TEMPLATE_NULL
	#define __STL_TEMPLATE_MULL template <>
#endif // !__STL_TEMPLATE_NULL


// 以下是一些自定义的全局信息：为了在实现自定义的 STL 工程中测试功能，而与现有的 MSVC STL 兼容
#define __C_FUNCTION__
