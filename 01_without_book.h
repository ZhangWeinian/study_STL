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

			template <typename U1, typename U2>
			explicit pair(const pair<U1, U2>& value): first(value.first), second(value.second)
			{
			}
		};

		template <typename FirstType, typename SecondType>
		inline bool operator==(const pair<FirstType, SecondType>& value1, const pair<FirstType, SecondType>& value2)
		{
			return value1.first == value2.first && value1.second == value2.second;
		}

		template <typename FirstType, typename SecondType>
		inline pair<FirstType, SecondType> make_pair(const FirstType& value1, const SecondType& value2)
		{
			return pair<FirstType, SecondType>(value1, value2);
		}


	} // namespace namespace_pair

	// 此处实现一些函数
	namespace namespace_function
	{
		template <typename InputIterator, typename T, typename Distance>
		class temporary_buffer
		{
		public:

			temporary_buffer(InputIterator, T, Distance n)
			{
				if (n != 0)
				{
					buf_begin = _cove_type(malloc(sizeof(T) * n), InputIterator);
				}
			}

		private:

			InputIterator buf_begin;
		};
	} // namespace namespace_function

	// 对外接口
	using namespace_pair::make_pair;
	using namespace_pair::pair;


#ifdef __ZH_NAMESPACE__
	#undef __ZH_NAMESPACE__
	#undef __ZH_ITER__
#endif // __ZH_NAMESPACE__

} // namespace zhang::without_book
