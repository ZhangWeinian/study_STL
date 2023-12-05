#pragma once

#include <string_view>
#include <type_traits>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <format>
#include <functional>
#include <iostream>
#include <iterator> // for MSVC STL _STD xxx_iterator_tag
#include <memory>
#include <new>
#include <string>
#include <vector>
#include <version>



#if !(__cplusplus < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 202002L
	#endif // !__HAS_CPP20

#elif !(__cplusplus < 201703L)

	#ifndef __HAS_CPP17
		#define __HAS_CPP17
	#endif // !__HAS_CPP17

#endif

#if !(_MSVC_LANG < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 202002L
	#endif // !__HAS_CPP20

#elif !(_MSVC_LANG < 201703L)

	#ifndef __HAS_CPP17
		#define __HAS_CPP17
	#endif // !__HAS_CPP17

#endif



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

#ifndef __begin_for_container
	#define __begin_for_container(cont) ::std::begin(cont)
#endif // !__begin_for_container

#ifndef __end_for_container
	#define __end_for_container(cont) ::std::end(cont)
#endif // !__end_for_container


#define __cove_type(cont, type)			static_cast<type>(cont)
#define __init_type(initCont, initType) __cove_type(initCont, initType)


// 以下全部都是 SGI STL 预定义全局变量
// 1、
inline constexpr auto __stl_threshold = 16;

// 2、
#ifndef __STL_TEMPLATE_NULL
	#define __STL_TEMPLATE_MULL template <>
#endif // !__STL_TEMPLATE_NULL


// 以下是一些自定义的全局信息：为了在实现自定义的 STL 工程中测试功能，而与现有的 MSVC STL 兼容
#define __C_FUNCTION__
