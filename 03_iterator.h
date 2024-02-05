#include "./00_basicFile.h"

#if __HAS_CPP20

__BEGIN_NAMESPACE_ZHANG

// 五种迭代器类型

	#ifndef __USE_ZH_TAG__


using input_iterator_tag		 = _STD			input_iterator_tag;
using output_iterator_tag		 = _STD		   output_iterator_tag;
using forward_iterator_tag		 = _STD		  forward_iterator_tag;
using bidirectional_iterator_tag = _STD bidirectional_iterator_tag;
using random_access_iterator_tag = _STD random_access_iterator_tag;
using contiguous_iterator_tag	 = _STD	   contiguous_iterator_tag;

	#else
struct input_iterator_tag
{
};

struct output_iterator_tag
{
};

struct forward_iterator_tag: input_iterator_tag, output_iterator_rag
{
};

struct bidirectional_iterator_tag: forward_iterator_tag
{
};

struct random_access_iterator_tag: bidirectional_iterator_tag
{
};

	#endif // !__use_myself_allocator__

// 为避免自定义时类型少写，自行开发的迭代器最好继承自如下所示的 _STD iterator
template <typename Category,
		  typename Type,
		  typename Distance	 = ptrdiff_t,
		  typename Pointer	 = Type*,
		  typename Reference = Type&>
struct iterator
{
	using iterator_category = Category;
	using value_type		= Type;
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
template <typename Type>
struct iterator_traits<Type*>
{
	using iterator_category = random_access_iterator_tag;
	using value_type		= Type;
	using difference_type	= ptrdiff_t;
	using pointer			= Type*;
	using reference			= Type&;
};

// 针对原生 pointer-to-const 而设计的偏特化版本
template <typename Type>
struct iterator_traits<const Type*>
{
	using iterator_category = random_access_iterator_tag;
	using value_type		= Type;
	using difference_type	= ptrdiff_t;
	using pointer			= const Type*;
	using reference			= const Type&;
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
	return __cove_type(0, iterator_traits<Iterator>::difference_type*);
}

// 此函数用于 -- 快速决定某个迭代器的 value type
template <typename Iterator>
inline typename iterator_traits<Iterator>::value_type* value_type(const Iterator&)
{
	return __cove_type(0, iterator_traits<Iterator>::value_type*);
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
inline typename iterator_traits<InputIterator>::difference_type distance(InputIterator first, InputIterator last)
{
	using category = typename iterator_traits<InputIterator>::iterator_category;

	return __distance(first, last, category());
}

template <typename InputIterator,
		  typename Distance> // 注意，这个函数书中未给出实现，但却有这样的用法，故在此处自定义实现
inline void distance(InputIterator first, InputIterator last, Distance& result)
{
	result = __cove_type(distance(first, last), Distance);
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
	__advance(i, n, iterator_category(i));
}

/*-----------------------------------------------------------------------------------------*/



// 以下是 SGI STL “私房菜” -- __type_traits

// __type_traits
template <typename type>
struct __type_traits
{
	using this_dummy_member_must_be_first = _STD true_type;
	/*
			 * 不要移除这个成员，它通知“有能力自动将 __type_traits
			 * 特化”的编译器，说：我们现在所看到的这个 __type_traits template 是特化的。
			 * 这是为了确保万一编译器也是用一个名为 __type_traits
			 * 而其实与此处定义并无关联的 template 时，所有事情都仍将继续运作
			 */

	/*
			   * 以下条件应该被遵守，因为编译器有可能自动为各型别产生专属的 __type_traits
			   * 特化版本：
			   *   -- 你可以重现排列以下成员的次序
			   *   -- 你可以移除以下任何成员
			   *   -- 不可以将以下成员重命名而没有改变编译器中的对应名称
			   *   -- 新加入的成员会被视为一般船员，除非你在编译器中加上适当支持
			*/
	using has_trivial_default_constructor = _STD false_type;
	using has_trivial_copy_constructor	  = _STD	false_type;
	using has_trivial_assignment_operator = _STD false_type;
	using has_trivial_destructor		  = _STD		  false_type;
	using is_POD_type					  = _STD					 false_type;
};

template <>
struct __type_traits<char>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<signed char>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<unsigned char>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<short>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<unsigned short>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<int>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<unsigned int>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<long>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<long long>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<unsigned long>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<float>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<double>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <>
struct __type_traits<long double>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

template <typename Type>
struct __type_traits<Type*>
{
	using has_trivial_default_constructor = _STD true_type;
	using has_trivial_copy_constructor	  = _STD	true_type;
	using has_trivial_assignment_operator = _STD true_type;
	using has_trivial_destructor		  = _STD		  true_type;
	using is_POD_type					  = _STD					 true_type;
};

__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
