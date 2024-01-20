#include "./00_basicFile.h"

#include <functional>

__BEGIN_NAMESPACE_ZHANG

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

template <typename Type>
struct plus: public binary_function<Type, Type, Type>
{
	Type operator()(const Type& x, const Type& y)
	{
		return x + y;
	}
};

template <typename Type>
struct miuns: public binary_function<Type, Type, Type>
{
	Type operator()(const Type& x, const Type& y)
	{
		return x - y;
	}
};

template <typename Type>
struct multiplies: public binary_function<Type, Type, Type>
{
	Type operator()(const Type& x, const Type& y)
	{
		return x * y;
	}
};

template <typename Type>
struct divides: public binary_function<Type, Type, Type>
{
	Type operator()(const Type& x, const Type& y)
	{
		return x / y;
	}
};

template <typename Type>
struct modulus: public binary_function<Type, Type, Type>
{
	Type operator()(const Type& x, const Type& y)
	{
		return x % y;
	}
};

template <typename Type>
struct negate: public unary_function<Type, Type>
{
	Type operator()(const Type& x)
	{
		return -x;
	}
};

// 关系运算类仿函数

template <typename Type>
struct equal_to: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x == y;
	}
};

template <typename Type>
struct not_equal_to: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x != y;
	}
};

template <typename Type>
struct greater: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x > y;
	}
};

template <typename Type>
struct less: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x < y;
	}
};

template <typename Type>
struct greater_equal: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x >= y;
	}
};

template <typename Type>
struct less_equal: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x <= y;
	}
};

// 逻辑运算类仿函数

template <typename Type>
struct logical_and: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x && y;
	}
};

template <typename Type>
struct logical_or: public binary_function<Type, Type, bool>
{
	bool operator()(const Type& x, const Type& y)
	{
		return x || y;
	}
};

template <typename Type>
struct logical_not: public unary_function<Type, bool>
{
	bool operator()(const Type& x)
	{
		return !x;
	}
};

/*-----------------------------------------------------------------------------------------*/


__END_NAMESPACE_ZHANG
