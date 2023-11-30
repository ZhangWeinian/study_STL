#pragma once

#include <algorithm>
#include <climits>
#include <iostream>
#include <iterator> // for MSVC STL _STD xxx_iterator_tag
#include <memory>
#include <new>
#include <version>


#ifndef _STD
	#define _STD ::std::
#endif // !_STD

#ifndef _move
	#define _move(cont) ::std::move(cont)
#endif // !_move



using _STD boolalpha;
using _STD cout;
using _STD cerr;
using _STD cin;
using _STD endl;


#define _cove_type(cont, type)		   static_cast<type>(cont)
#define _init_type(initCont, initType) _cove_type(initCont, initType)


// 以下全部都是 SGI STL 预定义全局变量
// 1、
inline constexpr auto __stl_threshold = 16;

// 2、
#ifndef __STL_TEMPLATE_NULL
	#define __STL_TEMPLATE_MULL template <>
#endif // !__STL_TEMPLATE_NULL


// 以下是一些自定义的全局信息：为了在实现自定义的 STL 工程中测试功能，而与现有的 MSVC STL 兼容
#define __C_FUNCTION__
