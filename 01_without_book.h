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


#ifdef __cpp20

	// 此处实现 print() （此函数不属于 STL ，只是基于 C++20 标准封装的 format() ）
	namespace namespace_print
	{
		inline void print(const char* msg, const char* arg)
		{
			_STD string fmt_msg { ::std::move(::std::vformat(msg, ::std::make_format_args(arg))) };

			fputs(fmt_msg.c_str(), stdout);
		}

		inline void print(const char* msg)
		{
			fputs(msg, stdout);
		}

		inline void print(const _STD string& msg)
		{
			fputs(msg.c_str(), stdout);
		}

		template <typename... Args>
		inline void print(const ::std::string_view msg, Args&&... args) noexcept
		{
			_STD string fmt_msg { ::std::move(::std::vformat(msg, ::std::make_format_args(args...))) };

			fputs(fmt_msg.c_str(), stdout);
		}

		template <typename InputIterator>
		inline void print(InputIterator first,
						  InputIterator last,
						  typename _STD iterator_traits<InputIterator>::difference_type max_distance = 1)
		{
			using difference_type = typename _STD iterator_traits<InputIterator>::difference_type;
			using value_type	  = typename _STD	   iterator_traits<InputIterator>::value_type;

			_STD string				  msg {};
			constexpr difference_type max_count = 1024 * 100;

			if constexpr ((_STD is_integral<value_type>::value) || (_STD is_floating_point<value_type>::value))
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
				for (difference_type __basic_distance { 1 }; (first != last) && (msg.size() < max_count);
					 ++first, ++__basic_distance)
				{
					_STD format_to(_STD back_inserter(msg), "{}\n", *first);
				}
			}

			fputs(msg.c_str(), stdout);
		}

		template <typename T>
		inline void print(const _STD vector<T, _STD allocator<T>>& nums,
						  typename _STD vector<T, _STD allocator<T>>::difference_type max_distance = 1)
		{
			namespace_print::print(nums.begin(), nums.end(), max_distance);
		}

		template <typename T1, typename T2>
		inline void print(const T1* first, const T2& count, const T2 max_distance = 15)
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

			fputs(msg.c_str(), stdout);
		}

#endif // __cpp20
	}  // namespace namespace_print

	// 对外接口
	using namespace_pair::make_pair;
	using namespace_pair::pair;

#ifdef __cpp20

	using namespace_print::print;

#endif // __cpp20


#ifdef __ZH_NAMESPACE__
	#undef __ZH_NAMESPACE__
	#undef __ZH_ITER__
#endif // __ZH_NAMESPACE__
} // namespace zhang::without_book
