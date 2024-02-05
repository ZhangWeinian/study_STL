#pragma once

#include <string_view>
#include <type_traits>

#include <algorithm>
#include <climits>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>
#include <version>



#if !(__cplusplus < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 __cplusplus
	#endif // !__HAS_CPP20

#elif !(_MSVC_LANG < 202002L)

	#ifndef __HAS_CPP20
		#define __HAS_CPP20 _MSVC_LANG
	#endif // !__HAS_CPP20

#endif



#if __HAS_CPP20

	#include <format>
	#include <ranges>
	#include <string_view>
	#include <compare>
	#include <concepts>


	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#ifndef _CSTD
		#define _CSTD ::
	#endif // !_CSTD

	#ifndef _RANGES
		#define _RANGES ::std::ranges::
	#endif // !_RANGES

	#ifndef _NODISCARD
		#define _NODISCARD [[nodiscard]]
	#endif // !_NODISCARD

	#ifndef _NORETURN
		#define _NORETURN [[noreturn]]
	#endif // !_NORETURN

	#ifndef __cove_type
		#define __cove_type(cont, type) static_cast<type>(cont)
	#endif // !__cove_type

	#ifndef __init_type
		#define __init_type(initCont, initType) __cove_type(initCont, initType)
	#endif // !__init_type


	#ifndef __STL_TEMPLATE_NULL
		#define __STL_TEMPLATE_MULL template <>
	#endif // !__STL_TEMPLATE_NULL

	#ifndef __BEGIN_NAMESPACE_ZHANG
		#define __BEGIN_NAMESPACE_ZHANG \
			namespace zhang             \
			{
	#endif // !__BEGIN_NAMESPACE_ZHANG

	#ifndef __END_NAMESPACE_ZHANG
		#define __END_NAMESPACE_ZHANG }
	#endif // !__END_NAMESPACE_ZHANG

	#ifndef __BEGIN_NEW_NAMESPACE
		#define __BEGIN_NEW_NAMESPACE(name) \
			namespace name                  \
			{
	#endif // !__BEGIN_NEW_NAMESPACE

	#ifndef __END_NEW_NAMESPACE
		#define __END_NEW_NAMESPACE(name) }
	#endif // !__END_NEW_NAMESPACE

	#ifndef __BEGIN_INLINE_NAMESPACE
		#define __BEGIN_INLINE_NAMESPACE(name) \
			inline namespace name              \
			{
	#endif // !__BEGIN_INLINE_NAMESPACE

	#ifndef __END_INLINE_NAMESPACE
		#define __END_INLINE_NAMESPACE(name) }
	#endif				// !__END_INLINE_NAMESPACE

__BEGIN_NAMESPACE_ZHANG // 此处开始，所有的代码都在命名空间 zhang 中

	#ifndef __stl_threshold
	template <typename Type>
	constexpr Type __stl_threshold = Type(16);
	#endif // !__stl_threshold


	#ifndef __max_msg_args_constant

template <typename Type>
constexpr inline Type __max_msg_args_constant = Type(128);

		#ifndef __limit_msg_args_constant

template <typename Type>
constexpr inline Type __limit_msg_args_constant = (__max_msg_args_constant<Type>) >> 1;

		#endif // !__limit_msg_args_constant

	#endif	   // !__max_msg_args_constant

	#ifndef __max_get_median_of_three_constant
template <typename Type>
constexpr inline Type __max_get_median_of_three_constant = Type(40);
	#endif // !__max_get_median_of_three_constant

template <typename>
constexpr inline bool __always_false = false;

// OutIter 或允许继承展开
template <typename Iterator>
concept __allow_inheriting_unwrap = (_STD is_same_v<Iterator, typename Iterator::_Prevent_inheriting_unwrap>);

// Wrapped 是弱展开
template <typename Wrapped>
concept __weakly_unwrappable = (__allow_inheriting_unwrap<_STD remove_cvref_t<Wrapped>>)&&(
	requires(Wrapped&& wrap) { _STD forward<Wrapped>(wrap)._Unwrapped(); });

// Sentinel 是弱展开哨兵
template <typename Sentinel>
concept __weakly_unwrappable_sentinel = __weakly_unwrappable<const _STD remove_reference_t<Sentinel>&>;

// OutIter 是弱展开迭代器
template <typename Iterator>
concept __weakly_unwrappable_iterator =
	(__weakly_unwrappable<Iterator>)&&(requires(Iterator&& iter, _STD remove_cvref_t<Iterator>& mut_iter) {
		mut_iter._Seek_to(_STD forward<Iterator>(iter)._Unwrapped());
	});

// 判断弱展开哨兵
template <typename Sentinel, typename Iterator>
concept __unwrappable_sentinel_for =
	(__weakly_unwrappable_sentinel<Sentinel>)&&(__weakly_unwrappable_iterator<Iterator>)&&(
		requires(Iterator&& iter, const _STD remove_reference_t<Sentinel>& sent) {
			{
				sent._Unwrapped()
			} -> _STD sentinel_for<decltype(_STD forward<Iterator>(iter)._Unwrapped())>;
		});

/*-----------------------------------------------------------------------------------------------------*/



template <typename Iterator>
concept __nothrow_unwrapped = requires(Iterator iter) {
	{
		iter._Unwrapped()
	} noexcept;
};

template <typename Iterator, typename Sentinel>
using __unwrap_iterator_type = _STD remove_cvref_t<decltype(__unwrap_iterator<Sentinel>(_STD declval<Iterator>()))>;

template <typename Sentinel, typename Iterator>
using __unwrap_sentinel_type = _STD remove_cvref_t<decltype(__unwrap_iterator<Iterator>(_STD declval<Sentinel>()))>;

template <_RANGES range Range>
using __unwrapped_iterator_type = __unwrap_iterator_type<_RANGES iterator_t<Range>, _RANGES sentinel_t<Range>>;

template <_RANGES range Range>
using __unwrapped_sentinel_type = __unwrap_sentinel_type<_RANGES sentinel_t<Range>, _RANGES iterator_t<Range>>;

// 此函数的作用是将 哨兵 展开为其 基础类型，即从一个迭代器返回一个裸指针
template <typename Iterator, typename Sentinel>
_NODISCARD constexpr decltype(auto)
	__unwrap_sentinel(Sentinel&& sent) noexcept((!__unwrappable_sentinel_for<Sentinel, Iterator>) ||
												(__nothrow_unwrapped<Sentinel>))
{
	if constexpr (_STD is_pointer_v<_STD remove_cvref_t<Sentinel>>) // 如果是指针，直接返回
	{
		return sent + 0;
	}
	else if constexpr (__unwrappable_sentinel_for<Sentinel, Iterator>) // 如果是弱展开哨兵，返回 _Unwrapped()
	{
		return static_cast<Sentinel&&>(sent)._Unwrapped();
	}
	else // 否则，返回 Sentinel&&
	{
		return static_cast<Sentinel&&>(sent);
	}
}

// 此函数的作用是将 迭代器 展开为其 基础类型，即从一个迭代器返回一个裸指针
template <typename Sentinel, typename Iterator>
_NODISCARD constexpr auto
	__unwrap_iterator(Iterator&& iter) noexcept((!__unwrappable_sentinel_for<Sentinel, Iterator>) ||
												(__nothrow_unwrapped<Iterator>))
{
	if constexpr (_STD is_pointer_v<_STD remove_cvref_t<Iterator>>)
	{
		return iter + 0;
	}
	else if constexpr (__unwrappable_sentinel_for<Sentinel, Iterator>)
	{
		return static_cast<Iterator&&>(iter)._Unwrapped();
	}
	else
	{
		return static_cast<Iterator&&>(iter);
	}
}

// 此函数的作用是将 迭代器 展开为其 基础类型，即从一个迭代器返回一个裸指针
template <_RANGES range Range, class Iterator>
_NODISCARD constexpr decltype(auto) __unwrap_range_iterator(Iterator&& iterator) noexcept(
	noexcept(__unwrap_iterator<_RANGES sentinel_t<Range>>(static_cast<Iterator&&>(iterator))))
{
	return __unwrap_iterator<_RANGES sentinel_t<Range>>(static_cast<Iterator&&>(iterator));
}

// 此函数的作用是将 哨兵 展开为其 基础类型，即从一个迭代器返回一个裸指针
template <_RANGES range Range, class Sentinel>
_NODISCARD constexpr decltype(auto) __unwrap_range_sentinel(Sentinel&& sentinel) noexcept(
	noexcept(__unwrap_sentinel<_RANGES iterator_t<Range>>(static_cast<Sentinel&&>(sentinel))))
{
	return __unwrap_sentinel<_RANGES iterator_t<Range>>(static_cast<Sentinel&&>(sentinel));
}

template <_STD forward_iterator Iterator, typename Sentinel>
	requires(_STD sentinel_for<_STD remove_cvref_t<Sentinel>, Iterator>)
_NODISCARD constexpr __unwrap_iterator_type<Iterator, Sentinel>
	__get_last_iterator_with_unwrapped(const __unwrap_iterator_type<Iterator, Sentinel>& first, Sentinel&& last)
{
	if constexpr (_STD
					  is_same_v<__unwrap_iterator_type<Iterator, Sentinel>, __unwrap_sentinel_type<Sentinel, Iterator>>)
	{
		return __unwrap_sentinel<Iterator>(_STD forward<Sentinel>(last));
	}
	else
	{
		return _RANGES next(first, __unwrap_sentinel<Iterator>(_STD forward<Sentinel>(last)));
	}
}

template <_RANGES forward_range Range>
_NODISCARD constexpr auto __get_last_iterator_with_unwrapped(Range& rng)
{
	if constexpr (_RANGES common_range<Range>)
	{
		if constexpr (_STD same_as<decltype(_RANGES end(rng)),
								   __unwrap_iterator_type<_RANGES iterator_t<Range>, _RANGES sentinel_t<Range>>>)
		{
			return _RANGES end(rng);
		}
		else
		{
			return __unwrap_range_sentinel<Range>(_RANGES end(rng));
		}
	}
	else if constexpr (_RANGES sized_range<Range>)
	{
		return _RANGES next(_RANGES begin(rng), _RANGES distance(rng));
	}
	else
	{
		return _RANGES next(_RANGES begin(rng), _RANGES end(rng));
	}
}

/*-----------------------------------------------------------------------------------------------------*/
template <class Type>
struct __choice_t
{
	Type strategy = Type {};
	bool no_throw = false;
};

namespace _Unchecked_begin
{
	template <typename Type>
	concept __has_member = requires(Type& t) {
		{
			t._Unchecked_begin()
		} -> _STD input_or_output_iterator;
	};

	template <class Type>
	concept __can_begin = requires(Type& t) { __unwrap_range_iterator<Type>(_RANGES begin(t)); };

	class UncheckBegin
	{
	private:

		enum class Start
		{
			None,
			Member,
			Unwrap
		};

		template <class Type>
		_NODISCARD static consteval __choice_t<Start> __choose() noexcept
		{
			if constexpr (__has_member<Type>)
			{
				return { Start::Member, noexcept(_STD _Fake_copy_init(_STD declval<Type>()._Unchecked_begin())) };
			}
			else if constexpr (__can_begin<Type>)
			{
				return { Start::Unwrap,
						 noexcept(_STD _Fake_copy_init(
							 __unwrap_range_iterator<Type>(_RANGES begin(_STD declval<Type>())))) };
			}
			else
			{
				return { Start::None };
			}
		}

		template <class Type>
		static constexpr __choice_t<Start> __choice = __choose<Type>();

	public:

		template <_RANGES _Should_range_access Type>
			requires(__choice<Type&>.strategy != Start::None)
		_NODISCARD constexpr auto operator()(Type&& value) const noexcept(__choice<Type&>.no_throw)
		{
			constexpr Start st = __choice<Type&>.strategy;

			if constexpr (st == Start::Member)
			{
				return value._Unchecked_begin();
			}
			else if constexpr (st == Start::Unwrap)
			{
				return __unwrap_range_iterator<Type>(_RANGES begin(value));
			}
			else
			{
				static_assert(__always_false<Type>, "Should be unreachable");
			}
		}
	};
} // namespace _Unchecked_begin

constexpr inline _Unchecked_begin::UncheckBegin ubegin;

namespace _Unchecked_end
{
	template <class Type>
	concept __has_member = _Unchecked_begin::__has_member<Type> && requires(Type& t) {
		t._Unchecked_begin();
		{
			t._Unchecked_end()
		} -> _STD sentinel_for<decltype(t._Unchecked_begin())>;
	};

	template <class Type>
	concept _Can_end = requires(Type& t) { __unwrap_range_sentinel<Type>(_RANGES end(t)); };

	class UncheckEnd
	{
	private:

		enum class Start
		{
			None,
			Member,
			Unwrap
		};

		template <class Type>
		_NODISCARD static consteval __choice_t<Start> __choose() noexcept
		{
			if constexpr (__has_member<Type>)
			{
				return { Start::Member, noexcept(_STD declval<Type>()._Unchecked_end()) };
			}
			else if constexpr (_Can_end<Type>)
			{
				return { Start::Unwrap, noexcept(__unwrap_range_sentinel<Type>(_RANGES end(_STD declval<Type>()))) };
			}
			else
			{
				return { Start::None };
			}
		}

		template <class Type>
		static constexpr __choice_t<Start> __choice = __choose<Type>();

	public:

		template <_RANGES _Should_range_access Type>
			requires(__choice<Type&>.strategy != Start::None)
		_NODISCARD constexpr auto operator()(Type&& value) const noexcept(__choice<Type&>.no_throw)
		{
			constexpr Start st = __choice<Type&>.strategy;

			if constexpr (st == Start::Member)
			{
				return value._Unchecked_end();
			}
			else if constexpr (st == Start::Unwrap)
			{
				return __unwrap_range_sentinel<Type>(_RANGES end(value));
			}
			else
			{
				static_assert(__always_false<Type>, "Should be unreachable");
			}
		}
	};
} // namespace _Unchecked_end

constexpr inline _Unchecked_end::UncheckEnd uend;

/*-----------------------------------------------------------------------------------------------------*/



template <typename Predicate>
struct __add_ref_for_function
{
	// 按值传递函数对象作为引用
	Predicate& pred;

	template <class... Args>
	constexpr decltype(auto) operator()(Args&&... args)
	{
		// forward function call operator
		if constexpr (_STD is_member_pointer_v<Predicate>)
		{
			return _STD invoke(pred, _STD forward<Args>(args)...);
		}
		else
		{
			return pred(_STD forward<Args>(args)...);
		}
	}
};

template <typename Predicate>
_NODISCARD constexpr auto __check_function(Predicate& pred) noexcept
{
	constexpr bool __pass_by_value = _STD conjunction_v<_STD bool_constant<sizeof(Predicate) <= sizeof(void*)>,
														_STD is_trivially_copy_constructible<Predicate>,
														_STD is_trivially_destructible<Predicate>>;
	if constexpr (__pass_by_value)
	{
		return pred;
	}
	else
	{
		return __add_ref_for_function<Predicate> { pred }; // 通过“引用”传递函子
	}
}

class __Not_quite_object
{
public:

	/*
	 * 库中的一些重载集具有这样的特性，即它们的组成函数模板对参数相关名称查找（ADL）不可见，
	 * 并且当通过非限定名称查找找到 ADL 时，它们会禁止 ADL 。此属性允许将这些重载集实现为函数对象。
	 * 我们从这种类型派生出这样的函数对象，以删除一些典型的对象行为，这有助于用户避免依赖于他们未指定的对象性。
	*/

	struct __Construct_tag
	{
		explicit __Construct_tag() = default;
	};

	__Not_quite_object() = delete;

	constexpr explicit __Not_quite_object(__Construct_tag) noexcept
	{
	}

	__Not_quite_object(const __Not_quite_object&)			 = delete;
	__Not_quite_object& operator=(const __Not_quite_object&) = delete;

	void operator&() const = delete;

protected:

	~__Not_quite_object() = default;
};

__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
