#pragma once

#include <algorithm>
#include <array>
#include <climits>
#include <iostream>
#include <iterator>
#include <vector>
#include <version>


#ifndef _STD
	#define _STD ::std::
#endif // !_STD


using _STD cout;
using _STD cerr;
using _STD endl;
using _STD cin;

using _STD boolalpha;

using _STD vector;
using _STD array;

using _STD pair;


#ifndef __myFun__

	#define _cove_type(cont, type)		   static_cast<type>(cont)
	#define _init_type(initCont, initType) _cove_type(initCont, initType)
	#define _def_vec(type)				   vector<type>

#endif // !__myFun__


// SGI STL 预定义全局变量
inline constexpr auto __stl_threshold = 16;
