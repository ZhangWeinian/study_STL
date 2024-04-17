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
#include <xmemory>


#ifndef _HAS_CXX20

	#if !(__cplusplus < 202'002L)

		#ifndef _HAS_CXX20
			#define _HAS_CXX20 __cplusplus
		#endif // !_HAS_CXX20

	#elif !(_MSVC_LANG < 202'002L)

		#ifndef _HAS_CXX20
			#define _HAS_CXX20 _MSVC_LANG
		#endif // !_HAS_CXX20

	#endif

#endif // !_HAS_CXX20



#if _HAS_CXX20

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

	#ifndef _RG
		#define _RG ::std::ranges::
	#endif // !_RG


	#ifndef _NODISCARD
		#define _NODISCARD [[nodiscard]]
	#endif // !_NODISCARD

	#ifndef _NORETURN
		#define _NORETURN [[noreturn]]
	#endif // !_NORETURN


	#ifndef __STL_TEMPLATE_NULL
		#define __STL_TEMPLATE_MULL template <>
	#endif // !__STL_TEMPLATE_NULL

	#ifndef _BEGIN_NAMESPACE_ZHANG
		#define _BEGIN_NAMESPACE_ZHANG \
			namespace zhang            \
			{
	#endif // !_BEGIN_NAMESPACE_ZHANG

	#ifndef _END_NAMESPACE_ZHANG
		#define _END_NAMESPACE_ZHANG }
	#endif // !_END_NAMESPACE_ZHANG

	#ifndef _BEGIN_NEW_NAMESPACE
		#define _BEGIN_NEW_NAMESPACE(name) \
			namespace name                 \
			{
	#endif // !_BEGIN_NEW_NAMESPACE

	#ifndef _END_NEW_NAMESPACE
		#define _END_NEW_NAMESPACE(name) }
	#endif // !_END_NEW_NAMESPACE

	#ifndef _BEGIN_INLINE_NAMESPACE
		#define _BEGIN_INLINE_NAMESPACE(name) \
			inline namespace name             \
			{
	#endif // !_BEGIN_INLINE_NAMESPACE

	#ifndef _END_INLINE_NAMESPACE
		#define _END_INLINE_NAMESPACE(name) }
	#endif // !_END_INLINE_NAMESPACE

/*------------------------------------------------------------------------------------------------*/



_BEGIN_NAMESPACE_ZHANG // 此处开始，所有的代码都在命名空间 zhang 中

	#ifndef _stl_threshold
	template <typename Type>
	constexpr Type _stl_threshold = Type(16);
	#endif // !_stl_threshold


	#ifndef _max_msg_args_constant

template <typename Type>
constexpr inline Type _max_msg_args_constant = Type(128);

		#ifndef _limit_msg_args_constant

template <typename Type>
constexpr inline Type _limit_msg_args_constant = (_max_msg_args_constant<Type>) >> 1;

		#endif // !_limit_msg_args_constant
	#endif	   // !_max_msg_args_constant

	#ifndef _max_get_median_of_three_constant
template <typename Type>
constexpr inline Type _max_get_median_of_three_constant = Type(40);
	#endif // !_max_get_median_of_three_constant

template <typename>
constexpr inline bool _always_false = false;

// OutIter 或允许继承展开
template <typename Iterator>
concept _allow_inheriting_unwrap = (_STD is_same_v<Iterator, typename Iterator::_Prevent_inheriting_unwrap>);

// Wrapped 是弱展开
template <typename Wrapped>
concept _weakly_unwrappable = (_allow_inheriting_unwrap<_STD remove_cvref_t<Wrapped>>) &&
							  (requires(Wrapped&& wrap) { _STD forward<Wrapped>(wrap)._Unwrapped(); });

// Sentinel 是弱展开哨兵
template <typename Sentinel>
concept _weakly_unwrappable_sentinel = _weakly_unwrappable<const _STD remove_reference_t<Sentinel>&>;

// OutIter 是弱展开迭代器
template <typename Iterator>
concept _weakly_unwrappable_iterator =
	(_weakly_unwrappable<Iterator>) && (requires(Iterator&& iter, _STD remove_cvref_t<Iterator>& mut_iter) {
											mut_iter._Seek_to(_STD forward<Iterator>(iter)._Unwrapped());
										});

// 判断弱展开哨兵
template <typename Sentinel, typename Iterator>
concept _unwrappable_sentinel_for =
	(_weakly_unwrappable_sentinel<Sentinel>) && (_weakly_unwrappable_iterator<Iterator>) &&
	(requires(Iterator&& iter, const _STD remove_reference_t<Sentinel>& sent) {
		 {
			 sent._Unwrapped()
		 } -> _STD sentinel_for<decltype(_STD forward<Iterator>(iter)._Unwrapped())>;
	 });

/*-----------------------------------------------------------------------------------------------------*/



template <typename Iterator>
concept _nothrow_unwrapped = requires(Iterator iter) {
								 {
									 iter._Unwrapped()
								 } noexcept;
							 };

template <typename Iterator, typename Sentinel>
using _unwrap_iterator_type = _STD remove_cvref_t<decltype(_unwrap_iterator<Sentinel>(_STD declval<Iterator>()))>;

template <typename Sentinel, typename Iterator>
using _unwrap_sentinel_type = _STD remove_cvref_t<decltype(_unwrap_iterator<Iterator>(_STD declval<Sentinel>()))>;

template <_RANGES range Range>
using _unwrapped_iterator_type = _unwrap_iterator_type<_RANGES iterator_t<Range>, _RANGES sentinel_t<Range>>;

template <_RANGES range Range>
using _unwrapped_sentinel_type = _unwrap_sentinel_type<_RANGES sentinel_t<Range>, _RANGES iterator_t<Range>>;

// 此函数的作用是将 哨兵 展开为其 基础类型，即从一个迭代器返回一个裸指针
template <typename Iterator, typename Sentinel>
_NODISCARD constexpr decltype(auto) _unwrap_sentinel(Sentinel&& sent)
	noexcept((!_unwrappable_sentinel_for<Sentinel, Iterator>) || (_nothrow_unwrapped<Sentinel>))
{
	if constexpr (_STD is_pointer_v<_STD remove_cvref_t<Sentinel>>) // 如果是指针，直接返回
	{
		return sent + 0;
	}
	else if constexpr (_unwrappable_sentinel_for<Sentinel, Iterator>) // 如果是弱展开哨兵，返回 _Unwrapped()
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
_NODISCARD constexpr auto _unwrap_iterator(Iterator&& iter)
	noexcept((!_unwrappable_sentinel_for<Sentinel, Iterator>) || (_nothrow_unwrapped<Iterator>))
{
	if constexpr (_STD is_pointer_v<_STD remove_cvref_t<Iterator>>)
	{
		return iter + 0;
	}
	else if constexpr (_unwrappable_sentinel_for<Sentinel, Iterator>)
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
_NODISCARD constexpr decltype(auto) _unwrap_range_iterator(Iterator&& iterator)
	noexcept(noexcept(_unwrap_iterator<_RANGES sentinel_t<Range>>(static_cast<Iterator&&>(iterator))))
{
	return _unwrap_iterator<_RANGES sentinel_t<Range>>(static_cast<Iterator&&>(iterator));
}

// 此函数的作用是将 哨兵 展开为其 基础类型，即从一个迭代器返回一个裸指针
template <_RANGES range Range, class Sentinel>
_NODISCARD constexpr decltype(auto) _unwrap_range_sentinel(Sentinel&& sentinel)
	noexcept(noexcept(_unwrap_sentinel<_RANGES iterator_t<Range>>(static_cast<Sentinel&&>(sentinel))))
{
	return _unwrap_sentinel<_RANGES iterator_t<Range>>(static_cast<Sentinel&&>(sentinel));
}

template <_STD forward_iterator Iterator, typename Sentinel>
	requires(_STD sentinel_for<_STD remove_cvref_t<Sentinel>, Iterator>)
_NODISCARD constexpr _unwrap_iterator_type<Iterator, Sentinel>
	_get_last_iterator_unwrapped(const _unwrap_iterator_type<Iterator, Sentinel>& first, Sentinel&& last)
{
	if constexpr (_STD is_same_v<_unwrap_iterator_type<Iterator, Sentinel>, _unwrap_sentinel_type<Sentinel, Iterator>>)
	{
		return _unwrap_sentinel<Iterator>(_STD forward<Sentinel>(last));
	}
	else
	{
		return _RANGES next(first, _unwrap_sentinel<Iterator>(_STD forward<Sentinel>(last)));
	}
}

template <_RANGES forward_range Range>
_NODISCARD constexpr auto _get_last_iterator_unwrapped(Range& rng)
{
	if constexpr (_RANGES common_range<Range>)
	{
		if constexpr (_STD same_as<decltype(_RANGES end(rng)),
								   _unwrap_iterator_type<_RANGES iterator_t<Range>, _RANGES sentinel_t<Range>>>)
		{
			return _RANGES end(rng);
		}
		else
		{
			return _unwrap_range_sentinel<Range>(_RANGES end(rng));
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
struct choice_t
{
	Type strategy = Type {};
	bool no_throw = false;
};

namespace _unchecked_begin
{
	template <typename Type>
	concept _has_member = requires(Type& t) {
							  {
								  t._unchecked_begin()
							  } -> _STD input_or_output_iterator;
						  };

	template <class Type>
	concept _can_begin = requires(Type& t) { _unwrap_range_iterator<Type>(_RANGES begin(t)); };

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
		_NODISCARD static consteval choice_t<Start> choose() noexcept
		{
			using enum Start;

			if constexpr (_has_member<Type>)
			{
				return { Member, noexcept(_STD _Fake_copy_init(_STD declval<Type>()._unchecked_begin())) };
			}
			else if constexpr (_can_begin<Type>)
			{
				return {
					Unwrap,
					noexcept(_STD _Fake_copy_init(_unwrap_range_iterator<Type>(_RANGES begin(_STD declval<Type>()))))
				};
			}
			else
			{
				return { None };
			}
		}

		template <class Type>
		static constexpr choice_t<Start> choice = choose<Type>();

	public:
		template <_RANGES _Should_range_access Type>
			requires(choice<Type&>.strategy != Start::None)
		_NODISCARD constexpr auto operator()(Type&& value) const noexcept(choice<Type&>.no_throw)
		{
			constexpr Start st = choice<Type&>.strategy;

			if constexpr (st == Start::Member)
			{
				return value._unchecked_begin();
			}
			else if constexpr (st == Start::Unwrap)
			{
				return _unwrap_range_iterator<Type>(_RANGES begin(value));
			}
			else
			{
				static_assert(_always_false<Type>, "Should be unreachable");
			}
		}
	};
} // namespace _unchecked_begin

constexpr inline _unchecked_begin::UncheckBegin __ubegin;

namespace _unchecked_end
{
	// _has_member 是一个概念，用于确定一个类型是否具有一个成员函数 _unchecked_end() 。
	// 如果存在这样的成员函数，它不会抛出异常，并且可以被ADL（参数依赖名称查找）发现。
	template <class Type>
	concept _has_member =
		(_unchecked_begin::_has_member<Type>) && (requires(Type& t) {
													  t._unchecked_begin();
													  {
														  t._unchecked_end()
													  } -> _STD sentinel_for<decltype(t._unchecked_begin())>;
												  });

	// _can_end 是一个概念，用于确定是否可以使用 _RANGES end()表达式来获取一个类型的结束 sentinel 。
	// 这通常用于那些不直接提供 _unchecked_end() 成员函数，但可以通过 ranges 库的 end() 函数来获取结束 sentinel 的类型。
	template <class Type>
	concept _can_end = requires(Type& t) { _unwrap_range_sentinel<Type>(_RANGES end(t)); };

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
		_NODISCARD static consteval choice_t<Start> choose() noexcept
		{
			using enum Start;

			if constexpr (_has_member<Type>)
			{
				return { Member, noexcept(_STD declval<Type>()._unchecked_end()) };
			}
			else if constexpr (_can_end<Type>)
			{
				return { Unwrap, noexcept(_unwrap_range_sentinel<Type>(_RANGES end(_STD declval<Type>()))) };
			}
			else
			{
				return { None };
			}
		}

		template <class Type>
		static constexpr choice_t<Start> choice = choose<Type>();

	public:
		template <_RANGES _Should_range_access Type>
			requires(choice<Type&>.strategy != Start::None)
		_NODISCARD constexpr auto operator()(Type&& value) const noexcept(choice<Type&>.no_throw)
		{
			constexpr Start st = choice<Type&>.strategy;

			if constexpr (st == Start::Member)
			{
				return value._unchecked_end();
			}
			else if constexpr (st == Start::Unwrap)
			{
				return _unwrap_range_sentinel<Type>(_RANGES end(value));
			}
			else
			{
				static_assert(_always_false<Type>, "Should be unreachable");
			}
		}
	};
} // namespace _unchecked_end

constexpr inline _unchecked_end::UncheckEnd __uend;

/*-----------------------------------------------------------------------------------------------------*/



template <typename Predicate>
struct
	_add_ref_for_function // 将谓词（Predicate）对象包装为一个引用传递的形式。这个结构体特别适用于那些不能按值传递的谓词对象，比如非平凡复制构造函数的类类型或者需要按引用传递的大型对象。
{
	Predicate&
		pred; // 非const左值引用，指向一个 Predicate 类型的对象。这意味着 _add_ref_for_function 对象将持有对原始谓词对象的一个引用。

	template <class... Args>
	constexpr auto operator()(Args&&... args)
	{
		if constexpr (_STD is_member_pointer_v<Predicate>) // 检查 Predicate 是否是一个成员指针类型
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
_NODISCARD constexpr auto _pass_function(Predicate& pred) noexcept
{
	constexpr bool _pass_by_value = _STD conjunction_v< // 将多个布尔常量表达式的结果逻辑与（AND）起来
		_STD bool_constant<sizeof(Predicate) <= sizeof(void*)>, // 检查 Predicate 的大小是否小于或等于指针的大小
		_STD is_trivially_copy_constructible<Predicate>, // 检查 Predicate是否具有平凡的复制构造函数
		_STD is_trivially_destructible<Predicate>		 // 检查 Predicate 是否具有平凡的析构函数
		>; // 如果谓词类型满足以上所有条件，_pass_by_value 为 true，表示可以按值传递。

	if constexpr (_pass_by_value)
	{
		return pred;
	}
	else
	{
		return _add_ref_for_function<Predicate> { pred }; // 通过“引用”传递函子
	}
}

class __Not_quite_object
{
public:
	/*
	* 它用于库中的一些重载集，这些重载集不希望参数通过ADL（参数依赖名称查找）被找到，因此它们禁止ADL。
	* 这种设计允许将这些重载集实现为函数对象，而不是常规对象。
	*
	* 库中的一些重载集具有这样的特性，即它们的组成函数模板对参数相关名称查找（ADL）不可见，
	* 并且当通过非限定名称查找找到 ADL 时，它们会禁止 ADL 。此属性允许将这些重载集实现为函数对象。
	* 我们从这种类型派生出这样的函数对象，以删除一些典型的对象行为（如默认构造、拷贝和赋值），这有助于用户避免依赖于他们未指定的对象性。
	*
	* 这种模式在库设计中很有用，特别是当你想提供一个轻量级的函数对象，它不应该被当作常规对象处理时。
	* 例如，它可能用于实现标准库中的某些算法或操作，其中对象的行为需要被精确控制，以避免意外的语义或性能开销。
	*/

	struct __Construct_tag
	{
		explicit __Construct_tag() = default;
	};

	constexpr explicit __Not_quite_object(__Construct_tag) noexcept {}

	__Not_quite_object()									 = delete;
	__Not_quite_object(const __Not_quite_object&)			 = delete;

	void				operator&() const					 = delete;
	__Not_quite_object& operator=(const __Not_quite_object&) = delete;

protected:
	~__Not_quite_object() = default;
};

_END_NAMESPACE_ZHANG

#endif // _HAS_CXX20
