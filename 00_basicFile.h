#pragma once

#include <algorithm>
#include <climits>
#include <iostream>
#include <iterator>
#include <memory>
#include <version>


#ifndef _STD
	#define _STD ::std::
#endif // !_STD


using _STD cout;
using _STD cerr;
using _STD endl;
using _STD cin;

using _STD boolalpha;


#ifndef __myFun__

	#define _cove_type(cont, type)		   static_cast<type>(cont)
	#define _init_type(initCont, initType) _cove_type(initCont, initType)
	#define _def_vec(type)				   vector<type>

#endif // !__myFun__



// 以下全部都是 SGI STL 预定义全局变量
// 1、
inline constexpr auto __stl_threshold = 16;

// 2、
#ifndef __STL_TEMPLATE_NULL
	#define __STL_TEMPLATE_MULL template <>
#endif // !__STL_TEMPLATE_NULL
