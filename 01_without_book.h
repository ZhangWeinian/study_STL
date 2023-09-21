#include "00_basicFile.h"

// 此处实现一些未在术中出现的内容
namespace zhang::without_book
{
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

	// 对外接口
	using namespace_pair::make_pair;
	using namespace_pair::pair;

} // namespace zhang::without_book
