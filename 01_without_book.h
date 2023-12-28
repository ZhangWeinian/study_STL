#pragma once

#include "00_basicFile.h"

// 此处实现一些未在书中出现的内容
namespace zhang::without_book
{
#ifdef __HAS_CPP20

	// 预定义一些用于 简写 和 标志识别 的宏
	#ifndef __zh_namespace

		#define __zh_namespace ::zhang::
		#define __zh_iter	   ::zhang::iterator::namespace_iterator::

	#endif // !__zh_namespace

	/*-----------------------------------------------------------------------------------------*/



	// 此处实现 pair<...>(...)
	namespace namespace_pair
	{
		template <typename FirstType, typename SecondType>
		struct pair
		{
			FirstType  first;
			SecondType second;

			pair(void): first(FirstType()), second(SecondType())
			{
			}

			pair(const FirstType& value1, const SecondType& value2): first(value1), second(value2)
			{
			}

			template <typename T>
			explicit pair(_STD initializer_list<T> info): first(*(info.begin())), second(*(info.begin() + 1))
			{
			}

			template <typename U1, typename U2>
			explicit pair(const pair<U1, U2>& value): first(value.first), second(value.second)
			{
			}

			template <typename FirstType, typename SecondType>
			inline bool operator==(const pair<FirstType, SecondType>& value) const noexcept
			{
				return __invoke(_RANGES equal_to {}, value.first, this->first) &&
					   __invoke(_RANGES equal_to {}, value.second, this->second);
			}
		};

		template <typename FirstType, typename SecondType>
		pair(FirstType, SecondType) -> pair<FirstType, SecondType>;

		template <typename FirstType, typename SecondType>
		inline pair<FirstType, SecondType> make_pair(const FirstType& value1, const SecondType& value2) noexcept
		{
			return pair<FirstType, SecondType>(value1, value2);
		}
	} // namespace namespace_pair

	  /*-----------------------------------------------------------------------------------------*/


	// 此处实现 print() （ 此函数不属于 STL ，只是基于 C++20 标准封装的 多功能打印函数 print() ）
	#ifdef __CPP20_PRINT

	namespace namespace_print
	{
		// 对于基础数据类型，采用如下的输出方式，称此方式为 基础输出方式
		struct __print_with_basic_data
		{
			template <typename T>
			inline void operator()(const T& msg) const
				noexcept(noexcept(_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}", msg)))
			{
				_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}", msg);
			}
		};

		inline constexpr __print_with_basic_data __default_print {};

		/*-----------------------------------------------------------------------------------------*/



		// 顶级输出语句之一。使用自定义打印函数 fun ，顺次输出参数包 args 中的所有参数
		template <typename Function, typename... Args>
		inline void __print_with_args(Function&& fun, Args&&... args) noexcept
		{
			(__invoke(fun, args), ...);
		}

		// 顶级输出语句之二。使用迭代器 first 和 last ，顺次输出 [first, last) 区间内的所有元素
		template <__is_input_iterator InputIterator, class Function = __print_with_basic_data>
		inline void __print_with_iter(InputIterator first, InputIterator last, Function fun)
		{
			using difference_type = __difference_type_for_iter<InputIterator>;
			using value_type	  = __value_type_for_iter<InputIterator>;

			_STD string data_tmp {};

			if constexpr (__not_basic_compound<value_type>) // 如果是复合类型，使用自定义打印函数 fun ，顺次输出所有元素
			{
				for (; first != last; ++first)
				{
					__invoke(fun, *first);
				}
			}
			else if constexpr ((_STD is_same_v<value_type, char>) ||
							   (_STD is_same_v<value_type, wchar_t>)) // 如果是字符类型，转为 string ，调用 fputs 输出
			{
				data_tmp = __move(_STD string(first, last));

				fputs(data_tmp.c_str(), stdout);
			}
			else if constexpr (_STD is_arithmetic_v<value_type>) // 如果是算术类型，适当美化后输出
			{
				constexpr difference_type max_distance { 15 };

				for (difference_type basic_distance { 1 }; first != last; ++first, ++basic_distance)
				{
					_STD format_to(_STD back_inserter(data_tmp), "{}\t", *first);

					if ((basic_distance % max_distance) == 0) // 每隔 max_distance 个元素输出一个换行符
					{
						if (first + 1 != last)
						{
							data_tmp += '\n';
						}

						fputs(data_tmp.c_str(), stdout);
						data_tmp.clear();
					}
				}

				if (!data_tmp.empty())
				{
					fputs(data_tmp.c_str(), stdout);
				}
			}

			fputs("\n", stdout);
		}

		// 顶级输出语句之三。首先尝试使用参包 args 格式化 msg ，若成功，则输出格式化后的字符串，否则顺次输出 T 和参数包 args 中的所有参数
		template <__basic_msg_type T, typename... Args>
		inline void __print_with_basic_mag(const T& msg, Args&&... args) noexcept
		{
			if constexpr (__is_basic_compound<T>) // 如果是基本类型的组合，直接输出
			{
				__print_with_args(__default_print, msg, _STD forward<Args>(args)...);
				fputs("\n", stdout);
			}
			else // 如果是 format() 格式，尝试格式化
			{
				_STD string data_tmp(msg);

				if (const auto& len_for_args { sizeof...(args) }; len_for_args == 0) // 如果没有参数，直接输出
				{
					data_tmp += '\n';
					fputs(data_tmp.c_str(), stdout);
				}
				else // 如果有参数，使用参包格式化
				{
					_STD string new_fmt_msg { __move(_STD vformat(msg, _STD make_format_args(args...))) };

					// 无论如何都输出一次
					fputs(new_fmt_msg.c_str(), stdout);

					if (data_tmp == new_fmt_msg) // 如果格式化后的字符串和原字符串相同
					{
						// 顺次输出参包的所有参数
						__print_with_args(__default_print, _STD forward<Args>(args)...);
					}

					fputs("\n", stdout);
				}
			}
		}

		// 针对 C风格指针 first 和 last ，按照是否可以计算距离，尝试使用不同的 顶级输出方式 输出
		template <__is_c_pointer C_pointer, class Function = __print_with_basic_data>
		inline void __print_with_c_pointer(C_pointer first, C_pointer last, Function fun = {})
		{
			using value_type = __value_type_for_iter<C_pointer>;

			if constexpr ((!(_STD is_same_v<value_type, char> || _STD is_same_v<value_type, wchar_t>)) &&
						  (noexcept(
							  last -
							  first))) // 如果是字符类型，且可以计算长度，则把它当作两个迭代器，使用 迭代器方式 输出
			{
				__print_with_iter(first, last, fun);
			}
			else // 如果是其他类型，或者不能计算长度，则顺次输出参包的所有参数
			{
				__print_with_args(__default_print, first, last);
			}
		}

		/*-----------------------------------------------------------------------------------------*/



		// 0.1、输出空行
		inline void print(void) noexcept
		{
			fputs("\n", stdout);
		}

		// 1.1、针对指向 基础类型数据的 迭代器 的一般泛化
		template <__is_input_iterator_without_c_pointer InputIterator, class Function = __print_with_basic_data>
			requires(__is_basic_compound<__value_type_for_iter<InputIterator>>)
		inline void print(InputIterator first, InputIterator last, Function fun = {})
		{
			__print_with_iter(first, last, fun);
		}

		// 1.2、针对指向 复合类型数据的 迭代器 的一般泛化
		template <__is_input_iterator_without_c_pointer InputIterator, class Function = __print_with_basic_data>
			requires(__not_basic_compound<__value_type_for_iter<InputIterator>>)
		inline void print(InputIterator first, InputIterator last, Function fun)
		{
			__print_with_iter(first, last, fun);
		}

		// 2.1、针对容纳 基础类型数据的 容器 的特化
		template <__is_range Range, class Function = __print_with_basic_data>
			requires(__is_basic_compound<__value_type_for_rg<Range>>)
		inline void print(const Range& con, Function fun = {})
		{
			__print_with_iter(__begin_for_container(con), __end_for_container(con), fun);
		}

		// 2.2、针对容纳 复合类型数据的 容器 的特化
		template <__is_range Range, class Function = __print_with_basic_data>
			requires(__not_basic_compound<__value_type_for_rg<Range>>)
		inline void print(const Range& con, Function fun)
		{
			__print_with_iter(__begin_for_container(con), __end_for_container(con), fun);
		}

		// 3.1 针对指向 基础类型数据的 C风格指针 的特化
		template <__is_c_pointer C_pointer, class Function = __print_with_basic_data>
			requires(__is_basic_compound<__value_type_for_iter<C_pointer>>)
		inline void print(C_pointer first, C_pointer last, Function fun = {})
		{
			__print_with_c_pointer(first, last, fun);
		}

		// 3.2 针对指向 复合类型数据的 C风格指针 的特化
		template <__is_c_pointer C_pointer, class Function = __print_with_basic_data>
			requires(__not_basic_compound<__value_type_for_iter<C_pointer>>)
		inline void print(C_pointer first, C_pointer last, Function fun)
		{
			__print_with_c_pointer(first, last, fun);
		}

		// 4.1、针对 format() 格式的一般泛化（右值）
		template <__basic_msg_type T, typename... Args>
		inline void print(T&& msg, Args&&... args) noexcept
		{
			__print_with_basic_mag(__move(msg), _STD forward<Args>(args)...);
		}

		/*-----------------------------------------------------------------------------------------*/
	}	   // namespace namespace_print

	#endif // __HAS_CPP20

	/*-----------------------------------------------------------------------------------------*/



	// 对外接口
	using namespace_pair::make_pair;
	using namespace_pair::pair;

	using namespace_print::print;


	/*-----------------------------------------------------------------------------------------*/



	#ifdef __zh_namespace
		#undef __zh_namespace
		#undef __zh_iter
	#endif // __zh_namespace

#endif	   // __HAS_CPP20
} // namespace zhang::without_book
