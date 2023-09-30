#include "00_basicFile.h"

#include <functional>

namespace zhang::functor
{
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
	namespace namespace_arithmetic
	{
		template <typename T>
		struct plus: public functor::binary_function<T, T, T>
		{
			T operator()(const T& x, const T& y)
			{
				return x + y;
			}
		};

		template <typename T>
		struct miuns: public functor::binary_function<T, T, T>
		{
			T operator()(const T& x, const T& y)
			{
				return x - y;
			}
		};

		template <typename T>
		struct multiplies: public functor::binary_function<T, T, T>
		{
			T operator()(const T& x, const T& y)
			{
				return x * y;
			}
		};

		template <typename T>
		struct divides: public functor::binary_function<T, T, T>
		{
			T operator()(const T& x, const T& y)
			{
				return x / y;
			}
		};

		template <typename T>
		struct modulus: public functor::binary_function<T, T, T>
		{
			T operator()(const T& x, const T& y)
			{
				return x % y;
			}
		};

		template <typename T>
		struct negate: public functor::unary_function<T, T>
		{
			T operator()(const T& x)
			{
				return -x;
			}
		};


	} // namespace namespace_arithmetic

	// 关系运算类仿函数
	namespace namespace_relational
	{
		template <typename T>
		struct equal_to: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x == y;
			}
		};

		template <typename T>
		struct not_equal_to: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x != y;
			}
		};

		template <typename T>
		struct greater: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x > y;
			}
		};

		template <typename T>
		struct less: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x < y;
			}
		};

		template <typename T>
		struct greater_equal: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x >= y;
			}
		};

		template <typename T>
		struct less_equal: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x <= y;
			}
		};
	} // namespace namespace_relational

	// 逻辑运算类仿函数
	namespace namespace_logical
	{
		template <typename T>
		struct logical_and: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x && y;
			}
		};

		template <typename T>
		struct logical_or: public functor::binary_function<T, T, bool>
		{
			bool operator()(const T& x, const T& y)
			{
				return x || y;
			}
		};

		template <typename T>
		struct logical_not: public functor::unary_function<T, bool>
		{
			bool operator()(const T& x)
			{
				return !x;
			}
		};
	} // namespace namespace_logical

	/*-----------------------------------------------------------------------------------------*/



	// 统一对外接口
	using namespace_arithmetic::divides;	   // 除
	using namespace_arithmetic::miuns;		   // 减
	using namespace_arithmetic::modulus;	   // 取余
	using namespace_arithmetic::multiplies;	   // 乘
	using namespace_arithmetic::negate;		   // 取反
	using namespace_arithmetic::plus;		   // 加

	using namespace_relational::equal_to;	   // 等于
	using namespace_relational::greater;	   // 大于
	using namespace_relational::greater_equal; // 大于等于
	using namespace_relational::less;		   // 小于
	using namespace_relational::less_equal;	   // 小于等于
	using namespace_relational::not_equal_to;  // 不等于

	using namespace_logical::logical_and;	   // 逻辑与
	using namespace_logical::logical_not;	   // 逻辑非
	using namespace_logical::logical_or;	   // 逻辑或

} // namespace zhang::functor
