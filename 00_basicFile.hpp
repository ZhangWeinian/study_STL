#pragma once

#include <climits>
#include <iostream>
#include <vector>
#include <version>


#ifndef _STD
	#define _STD ::std::
#endif // !_STD

using _STD cout;
using _STD cerr;
using _STD endl;
using _STD cin;

using _STD vector;
using _STD array;


#ifndef __myFun__
	#define _cove_type(cont, type) static_cast<type>(cont)
	#define _def_vec(type)		   vector<type>
#endif // !__myFun__
