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
			inline bool operator==(const pair<FirstType, SecondType>& value) const
			{
				return (value.first == this->first) && (value.second == this->second);
			}
		};

		template <typename FirstType, typename SecondType>
		pair(FirstType, SecondType) -> pair<FirstType, SecondType>;

		template <typename FirstType, typename SecondType>
		inline pair<FirstType, SecondType> make_pair(const FirstType& value1, const SecondType& value2)
		{
			return pair<FirstType, SecondType>(value1, value2);
		}
	} // namespace namespace_pair

	  /*-----------------------------------------------------------------------------------------*/


	// 此处实现 print() （ 此函数不属于 STL ，只是基于 C++20 标准封装的 多功能输出函数 print() ）
	#ifdef __CPP20_PRINT

	namespace namespace_print
	{
		template <__is_iterator_or_c_pointer InputIterator>
		inline void __print_with_iter(InputIterator first, InputIterator last) noexcept
		{
			using difference_type = __difference_type_for_iter<InputIterator>;
			using value_type	  = __value_type_for_iter<InputIterator>;

			_STD string				  msg {};
			difference_type			  max_distance { 1 };
			constexpr difference_type max_count { 1024 * 100 };

			if constexpr ((_STD is_same_v<value_type, char>) || (_STD is_same_v<value_type, wchar_t>))
			{
				msg = _STD string(first, last);
			}
			else if constexpr (_STD is_arithmetic_v<value_type>)
			{
				if (max_distance == 1)
				{
					max_distance = 15;
				}

				for (difference_type basic_distance { 1 }; (first + 1 != last) && (msg.size() < max_count);
					 ++first, ++basic_distance)
				{
					_STD format_to(_STD back_inserter(msg), "{}\t", *first);

					if (basic_distance % max_distance == 0)
					{
						msg += '\n';
					}
				}
				_STD format_to(_STD back_inserter(msg), "{}\t", *first);
			}
			else
			{
				for (; (first != last) && (msg.size() < max_count); ++first)
				{
					_STD format_to(_STD back_inserter(msg), "{}\n", *first);
				}
			}

			fputs(msg.c_str(), stdout);
		}

		template <__basic_msg_type T, typename... Args>
		inline void __print_with_basic_mag(const T& msg, Args&&... args) noexcept
		{
			if constexpr (__is_basic_compound<T>)
			{
				_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}", msg);
			}
			else
			{
				::std::string fmt_msg { __move(::std::vformat(msg, ::std::make_format_args(args...))) };
				fputs(fmt_msg.c_str(), stdout);
			}
		}

		template <__is_c_pointer C_pointer>
		inline void __print_with_c_pointer(C_pointer first, C_pointer last) noexcept
		{
			using value_type = __value_type_for_iter<C_pointer>;

			if constexpr ((!(_STD is_same_v<value_type, char> || _STD is_same_v<value_type, wchar_t>)) &&
						  (noexcept(last - first)))
			{
				__print_with_iter(first, last);
			}
			else
			{
				__print_with_basic_mag(first, _STD forward<C_pointer>(last));
			}
		}

		// 输出空行
		inline void print(void) noexcept
		{
			fputs("\n", stdout);
		}

		// 输出空行
		inline void println(void) noexcept
		{
			print();
		}

		// 1.1、针对 迭代器 的一般泛化
		template <__is_iterator_without_c_pointer InputIterator>
		inline void print(InputIterator first, InputIterator last) noexcept
		{
			__print_with_iter(first, last);
		}

		// 1.2、针对 迭代器 的一般泛化的 println()
		template <__is_iterator_without_c_pointer InputIterator>
		inline void println(InputIterator first, InputIterator last) noexcept
		{
			__print_with_iter(first, last);
			print();
		}

		// 2.1、针对 容器 的特化
		template <__is_container_or_c_array Container>
		inline void print(const Container& con) noexcept
		{
			__print_with_iter(__begin_for_container(con), __end_for_container(con));
		}

		// 2.2、针对 容器 的特化的 println()
		template <__is_container_or_c_array Container>
		inline void println(const Container& con) noexcept
		{
			__print_with_iter(__begin_for_container(con), __end_for_container(con));
			print();
		}

		// 3.1 针对 C风格指针 的特化
		template <__is_c_pointer C_pointer>
		inline void print(C_pointer first, C_pointer last) noexcept
		{
			__print_with_c_pointer(first, last);
		}

		// 3.2 针对 C风格指针 的特化的 println()
		template <__is_c_pointer C_pointer>
		inline void println(C_pointer first, C_pointer last) noexcept
		{
			__print_with_c_pointer(first, last);
			print();
		}

		// 4.1、针对 format() 格式的一般泛化（右值）
		template <__basic_msg_type T, typename... Args>
		inline void print(T&& msg, Args&&... args) noexcept
		{
			__print_with_basic_mag(__move(msg), _STD forward<Args>(args)...);
		}

		// 4.1、针对 format() 格式的一般泛化（左值）
		template <__basic_msg_type T, typename... Args>
		inline void print(const T& msg, Args&&... args) noexcept
		{
			__print_with_basic_mag(msg, _STD forward<Args>(args)...);
		}

		// 4.2、针对 format() 格式的一般泛化（右值）的 println()
		template <__basic_msg_type T, typename... Args>
		inline void println(T&& msg, Args&&... args) noexcept
		{
			__print_with_basic_mag(__move(msg), _STD forward<Args>(args)...);
			print();
		}

		// 4.2、针对 format() 格式的一般泛化（左值）的 println()
		template <__basic_msg_type T, typename... Args>
		inline void println(const T& msg, Args&&... args) noexcept
		{
			__print_with_basic_mag(msg, _STD forward<Args>(args)...);
			print();
		}
	}	   // namespace namespace_print

	#endif // __HAS_CPP20

	/*-----------------------------------------------------------------------------------------*/



	// 对外接口
	using namespace_pair::make_pair;
	using namespace_pair::pair;

	using namespace_print::print;
	using namespace_print::println;

	/*-----------------------------------------------------------------------------------------*/



	#ifdef __zh_namespace
		#undef __zh_namespace
		#undef __zh_iter
	#endif // __zh_namespace

#endif	   // __HAS_CPP20
} // namespace zhang::without_book
