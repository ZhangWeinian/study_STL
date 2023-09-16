#pragma once


#ifndef _STD
	#define _STD ::std::
#endif // !_STD


#if __has_include(<iostream>)
using _STD cout;
using _STD cerr;
using _STD endl;
using _STD cin;
#endif

#if __has_include(<vector>)
using _STD vector;
#endif

#if __has_include(<array>)
using _STD array;
#endif


#ifndef __myFun__
	#define _cove_type(cont, type) static_cast<type>(cont)
	#define _def_vec(type)		   vector<type>
#endif // !__myFun__
