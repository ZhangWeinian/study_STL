#pragma once

#include "00_basicFile.h"

// 此处实现一些未在书中出现的内容
namespace zhang::without_book
{
	// 预定义一些用于 简写 和 标志识别 的宏
#ifndef __ZH_NAMESPACE__

	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#define __ZH_NAMESPACE__ ::zhang::
	#define __ZH_ITER__		 ::zhang::iterator::namespace_iterator::

#endif // !__ZH_NAMESPACE__


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
		inline pair<FirstType, SecondType> make_pair(const FirstType& value1, const SecondType& value2)
		{
			return pair<FirstType, SecondType>(value1, value2);
		}
	} // namespace namespace_pair

#if defined(__HASCPP20) && __has_include(<format>)

	// 此处实现 print() （此函数不属于 STL ，只是基于 C++20 标准封装的 多功能输出函数 print() ）
	namespace namespace_print
	{
		template <typename T>
		concept __is_iterator = requires(T p) {
			_STD input_iterator<T>;
			typename T::difference_type;
			++p;
		};

		template <typename T>
		concept __is_c_array = _STD is_array_v<T>;

		template <typename T>
		concept __is_pointer = _STD is_pointer_v<T>;

		template <typename T>
		concept __is_basic_compound = !(_STD is_compound_v<T>);

		template <typename T>
		concept __is_containers = requires(T p) {
			typename T::value_type;
			p.begin();
			p.end();
		};

		template <typename T>
		concept __basic_msg_type = (__is_basic_compound<T>) || requires(T msg) { _STD string(msg); } ||
								   requires(T msg) { _STD string_view(msg); };

		// 输出空行
		inline void print(void) noexcept
		{
			_STD cout << _STD endl;
		}

		inline void println(void) noexcept
		{
			_STD cout << _STD endl;
		}

		// 1.1、针对 C 风格数组 <指针，距离> 的特化
		template <typename T1, typename T2 = size_t>
			requires(namespace_print::__is_c_array<T1> && (namespace_print::__is_basic_compound<T2>))
		inline void print(const T1& first, const T2& count, const T2& max_distance = 15) noexcept
		{
			_STD string msg {};
			T2			__basic_count { 0 };

			for (T2 __basic_distance { 1 }; __basic_count + 1 != count; ++__basic_count, ++__basic_distance)
			{
				_STD format_to(_STD back_inserter(msg), "{}\t", *(first + __basic_count));

				if (__basic_distance % max_distance == 0)
				{
					msg += '\n';
				}
			}
			_STD format_to(_STD back_inserter(msg), "{}\t", *(first + __basic_count));

			_STD cout << _STD vformat(msg, _move(_STD make_format_args()));
		}

		// 1.2、针对 C 风格数组 <指针，指针> 的特化
		template <typename T1, typename T2, typename T3 = size_t>
			requires(namespace_print::__is_c_array<T1> && (namespace_print::__is_pointer<T2>))
		inline void print(const T1& first, const T2& last, const T3& max_distance = 15)
		{
			namespace_print::print(first, _STD distance(first, last), max_distance);
		}

		// 1.3、针对 C 风格数组 <指针，距离> 的特化的 println()
		template <typename T, typename T2 = size_t>
			requires(namespace_print::__is_c_array<T>)
		inline void println(const T& first, const T2& count, const T2 max_distance = 15) noexcept
		{
			namespace_print::print(first, count, max_distance);
			namespace_print::print();
		}

		// 1.4、针对 C 风格数组 <指针，指针> 的特化的println()
		template <typename T1, typename T2, typename T3 = size_t>
			requires(namespace_print::__is_c_array<T1> && (namespace_print::__is_pointer<T2>))
		inline void println(const T1& first, const T2& last, const T3& max_distance = 15)
		{
			namespace_print::print(first, _STD distance(first, last), max_distance);
			namespace_print::print();
		}

		// 2.1、针对 迭代器 的特化
		template <typename InputIterator>
			requires(namespace_print::__is_iterator<InputIterator>)
		inline void print(InputIterator first,
						  InputIterator last,
						  typename _STD iterator_traits<InputIterator>::difference_type max_distance = 1) noexcept
		{
			using difference_type = typename _STD iterator_traits<InputIterator>::difference_type;
			using value_type	  = typename _STD	   iterator_traits<InputIterator>::value_type;

			_STD string				  msg {};
			constexpr difference_type max_count = 1024 * 100;

			if constexpr (_STD is_arithmetic_v<value_type>)
			{
				if (max_distance == 1)
				{
					max_distance = 15;
				}

				for (difference_type __basic_distance { 1 }; (first + 1 != last) && (msg.size() < max_count);
					 ++first, ++__basic_distance)
				{
					_STD format_to(_STD back_inserter(msg), "{}\t", *first);

					if (__basic_distance % max_distance == 0)
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

			_STD cout << _STD vformat(msg, _move(_STD make_format_args()));
		}

		// 2.2、针对 迭代器 的特化的 println()
		template <typename InputIterator>
			requires(namespace_print::__is_iterator<InputIterator>)
		inline void println(InputIterator first,
							InputIterator last,
							typename _STD iterator_traits<InputIterator>::difference_type max_distance = 1) noexcept
		{
			namespace_print::print(first, last, max_distance);
			namespace_print::print();
		}

		// 3.1、针对 容器 的特化
		template <typename T, typename Difference_type = size_t>
			requires(namespace_print::__is_containers<T>)
		inline void print(const T& con, Difference_type max_distance = 1) noexcept
		{
			namespace_print::print(_begin(con), _end(con), max_distance);
		}

		// 3.2、针对 容器 的特化的 println()
		template <typename T, typename Difference_type = size_t>
			requires(namespace_print::__is_containers<T>)
		inline void println(const T& con, Difference_type max_distance = 1) noexcept
		{
			namespace_print::print(con, max_distance);
			namespace_print::print();
		}

		// 4.1、一般泛化
		template <typename T, typename... Args>
			requires(namespace_print::__basic_msg_type<T>)
		inline void print(const T& msg, Args&&... args) noexcept
		{
			if constexpr (namespace_print::__is_basic_compound<T>)
			{
				_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}", msg);
			}
			else
			{
				::std::string fmt_msg { _move(::std::vformat(msg, ::std::make_format_args(args...))) };
				fputs(fmt_msg.c_str(), stdout);
			}
		}

		// 4.2、一般泛化模式下的 println()
		template <typename T, typename... Args>
			requires(namespace_print::__basic_msg_type<T>)
		inline void println(const T& msg, Args&&... args) noexcept
		{
			namespace_print::print(msg, args...);
			namespace_print::print();
		}
	}  // namespace namespace_print
#endif // __HASCPP20

	// 对外接口
	using namespace_pair::make_pair;
	using namespace_pair::pair;

#if defined(__HASCPP20) && __has_include(<format>)

	using namespace_print::print;
	using namespace_print::println;

#endif // __HASCPP20


#ifdef __ZH_NAMESPACE__
	#undef __ZH_NAMESPACE__
	#undef __ZH_ITER__
#endif // __ZH_NAMESPACE__
} // namespace zhang::without_book
