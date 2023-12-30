#include "00_basicFile.h"

#include <functional>

__BEGIN_NEW_NP(np_functor)

// 一元谓词
template <typename Arg, typename Result>
struct unary_function
{
	using argument_type = Arg;
	using result_type	= Result;
};

// 二元谓词
template <typename Arg1, typename Arg2, typename Result>
struct binary_function
{
	using first_argument_type  = Arg1;
	using second_argument_type = Arg2;
	using result_type		   = Result;
};


// 算术类仿函数
__BEGIN_NEW_NP(np_arithmetic)

template <typename T>
struct plus: public np_functor::binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)
	{
		return x + y;
	}
};

template <typename T>
struct miuns: public np_functor::binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)
	{
		return x - y;
	}
};

template <typename T>
struct multiplies: public np_functor::binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)
	{
		return x * y;
	}
};

template <typename T>
struct divides: public np_functor::binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)
	{
		return x / y;
	}
};

template <typename T>
struct modulus: public np_functor::binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)
	{
		return x % y;
	}
};

template <typename T>
struct negate: public np_functor::unary_function<T, T>
{
	T operator()(const T& x)
	{
		return -x;
	}
};

__END_NEW_NP(np_arithmetic)


// 关系运算类仿函数
__BEGIN_NEW_NP(np_relational)

template <typename T>
struct equal_to: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x == y;
	}
};

template <typename T>
struct not_equal_to: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x != y;
	}
};

template <typename T>
struct greater: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x > y;
	}
};

template <typename T>
struct less: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x < y;
	}
};

template <typename T>
struct greater_equal: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x >= y;
	}
};

template <typename T>
struct less_equal: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x <= y;
	}
};


__END_NEW_NP(np_relational)

// 逻辑运算类仿函数
__BEGIN_NEW_NP(np_logical)

template <typename T>
struct logical_and: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x && y;
	}
};

template <typename T>
struct logical_or: public np_functor::binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x || y;
	}
};

template <typename T>
struct logical_not: public np_functor::unary_function<T, bool>
{
	bool operator()(const T& x)
	{
		return !x;
	}
};


__END_NEW_NP(np_logical)

/*-----------------------------------------------------------------------------------------*/



// 统一对外接口
using np_arithmetic::divides;		// 除
using np_arithmetic::miuns;			// 减
using np_arithmetic::modulus;		// 取余
using np_arithmetic::multiplies;	// 乘
using np_arithmetic::negate;		// 取反
using np_arithmetic::plus;			// 加

using np_relational::equal_to;		// 等于
using np_relational::greater;		// 大于
using np_relational::greater_equal; // 大于等于
using np_relational::less;			// 小于
using np_relational::less_equal;	// 小于等于
using np_relational::not_equal_to;	// 不等于

using np_logical::logical_and;		// 逻辑与
using np_logical::logical_not;		// 逻辑非
using np_logical::logical_or;		// 逻辑或

__END_NEW_NP(np_functor)
