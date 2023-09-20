#pragma once

#include <algorithm>
#include <climits>
#include <iostream>
#include <iterator>
#include <vector>
#include <version>


/* myself function */
#ifndef _STD
	#define _STD ::std::
#endif // !_STD

#ifndef _ZH
	#define _ZH ::zhang::
#endif // !_ZH


using _STD cout;
using _STD cerr;
using _STD endl;
using _STD cin;

using _STD vector;
using _STD array;


#ifndef __myFun__

	#define _cove_type(cont, type)		   static_cast<type>(cont)
	#define _init_type(initCont, initType) _cove_type(initCont, initType)
	#define _def_vec(type)				   vector<type>

#endif // !__myFun__


// SGI STL 预定义全局变量
constexpr auto __stl_threshold = 16;
