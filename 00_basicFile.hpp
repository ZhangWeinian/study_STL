#pragma once


#ifndef _STD
	#define _STD ::std::
#endif // !_STD


#ifndef __myFun__
	#define _cove_type(cont, type) static_cast<type>(cont)
#endif // !__myFun__


#if __has_include(<iostream>)
using _STD cout;
using _STD cerr;
using _STD endl;
using _STD cin;
#endif
