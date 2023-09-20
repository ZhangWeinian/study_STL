#include "00_basicFile.h"

namespace zhang::iterator
{
	namespace namespace_iterator
	{
		// 五种迭代器类型
		struct input_iterator_tag
		{
		};

		struct output_inerator_tag
		{
		};

		struct forward_iterator_tag
		{
		};

		struct bidirectional_iterator_tag
		{
		};

		struct random_access_iterator_tag
		{
		};

		// 为避免自定义时类型少写，自行开发的迭代器最好继承自如下所示的 _STD iterator
		template <typename Category,
				  typename T,
				  typename Distance	 = ptrdiff_t,
				  typename Pointer	 = T*,
				  typename Reference = T&>
		struct iterator
		{
			using iterator_category = Category;
			using value_type		= T;
			using difference_type	= Distance;
			using pointer			= Pointer;
			using reference			= Reference;
		};

		// 萃取 -- traits
		template <typename Iterator>
		struct iterator_traits
		{
			using iterator_category = typename Iterator::iterator_category;
			using value_type		= typename Iterator::value_type;
			using difference_type	= typename Iterator::difference_type;
			using pointer			= typename Iterator::pointer;
			using reference			= typename Iterator::reference;
		};

		// 针对原生指针而设计的 traits 偏特化版本
		template <typename T>
		struct iterator_traits<T*>
		{
			using iterator_category = random_access_iterator_tag;
			using value_type		= T;
			using difference_type	= ptrdiff_t;
			using pointer			= T*;
			using reference			= T&;
		};

		// 针对原生 pointer-to-const 而设计的偏特化版本
		template <typename T>
		struct iterator_traits<const T*>
		{
			using iterator_category = random_access_iterator_tag;
			using value_type		= T;
			using difference_type	= ptrdiff_t;
			using pointer			= const T*;
			using reference			= const T&;
		};

		// 此函数用于 -- 快速决定某个迭代器的类型（category）
		template <typename Iterator>
		inline typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator&)
		{
			using category = typename iterator_traits<Iterator>::iterator_category;
			return category();
		}

		// 此函数用于 -- 快速决定某个迭代器的 distance type
		template <typename Iterator>
		inline typename iterator_traits<Iterator>::difference_type* distance_type(const Iterator&)
		{
			return _cove_type(0, iterator_traits<Iterator>::difference_type*);
		}

		// 此函数用于 -- 快速决定某个迭代器的 value type
		template <typename Iterator>
		inline typename iterator_traits<Iterator>::value_type* value_type(const Iterator&)
		{
			return _cove_type(0, iterator_traits<Iterator>::value_type*);
		}

		// 以下是整组 distance 函数
		template <typename InputIterator>
		inline typename iterator_traits<InputIterator>::difference_type
			__distance(InputIterator first, InputIterator last, input_iterator_tag)
		{
			typename iterator_traits<InputIterator>::difference_type n = 0;

			while (first != last)
			{
				++first;
				++n;
			}

			return n;
		}

		template <typename RandomAccessIterator>
		inline typename iterator_traits<RandomAccessIterator>::difference_type
			__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
		{
			return last - first;
		}

		template <typename InputIterator>
		inline typename iterator_traits<InputIterator>::difference_type distance(InputIterator first,
																				 InputIterator last)
		{
			using category = typename iterator_traits<InputIterator>::iterator_category;

			return namespace_iterator::__distance(first, last, category());
		}

		// 以下是整组 advance 函数
		template <typename InputIterator, typename Distance>
		inline void __advance(InputIterator& i, Distance n, input_iterator_tag)
		{
			while (n--)
			{
				++i;
			}
		}

		template <typename BidirectionalIterator, typename Distance>
		inline void __advance(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
		{
			if (n >= 0)
			{
				while (n--)
				{
					++i;
				}
			}
			else
			{
				while (n++)
				{
					--i;
				}
			}
		}

		template <typename RandomAccessIterator, typename Distance>
		inline void __advance(RandomAccessIterator& i, Distance n, random_access_iterator_tag)
		{
			i += n;
		}

		template <typename InputIterator, typename Distance>
		inline void advance(InputIterator& i, Distance n)
		{
			namespace_iterator::__advance(i, n, iterator_category(i));
		}
	} // namespace namespace_iterator

	// 对外接口
	using namespace_iterator::advance;
	using namespace_iterator::bidirectional_iterator_tag;
	using namespace_iterator::distance;
	using namespace_iterator::distance_type;
	using namespace_iterator::forward_iterator_tag;
	using namespace_iterator::input_iterator_tag;
	using namespace_iterator::iterator;
	using namespace_iterator::iterator_category;
	using namespace_iterator::iterator_traits;
	using namespace_iterator::output_inerator_tag;
	using namespace_iterator::random_access_iterator_tag;
	using namespace_iterator::value_type;

} // namespace zhang::iterator
