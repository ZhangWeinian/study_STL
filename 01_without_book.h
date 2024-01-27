#pragma once

#include "./00_basicFile.h"


#ifdef __HAS_CPP20

__BEGIN_NAMESPACE_ZHANG

// 此处实现 pair<...>(...)
template <typename FirstType, typename SecondType>
struct pair
{
	FirstType  first_value;
	SecondType second_value;

	pair(void): first_value(FirstType()), second_value(SecondType())
	{
	}

	pair(const FirstType& value1, const SecondType& value2): first_value(value1), second_value(value2)
	{
	}

	template <typename Type>
	explicit pair(_STD initializer_list<Type> init_list):
	first_value(*(init_list.begin())), second_value(*(init_list.begin() + 1))
	{
	}

	template <typename Type1, typename Type2>
	explicit pair(const pair<Type1, Type2>& value): first_value(value.first), second_value(value.second)
	{
	}

	template <typename Type1, typename Type2>
	constexpr bool operator==(const pair<Type1, Type2>& p) const noexcept
	{
		return (_STD invoke(_RANGES equal_to {}, p.first_value, this->first_value) &&
				_STD invoke(_RANGES equal_to {}, p.second_value, this->second_value));
	}
};

// 此处实现 make_pair(...)
template <typename FirstType, typename SecondType>
static constexpr pair<FirstType, SecondType> make_pair(const FirstType& value1, const SecondType& value2) noexcept
{
	return pair<FirstType, SecondType>(value1, value2);
}

// pair 推断指引
template <typename FirstType, typename SecondType>
pair(FirstType, SecondType) -> pair<FirstType, SecondType>;

/*-----------------------------------------------------------------------------------------------------*/

// 此处实现 print<...>(...)

// 以下是准备工作

// 工作1、定义基础输出类型
template <typename MsgType>
concept __basic_msg_type = (_STD is_fundamental_v<_STD remove_cvref_t<MsgType>>) ||
						   (requires(_STD remove_cvref_t<MsgType> msg) { noexcept(_STD string(msg)); }) ||
						   (requires(_STD remove_cvref_t<MsgType> msg) { noexcept(_STD wstring(msg)); });

// 工作2、定义：使用默认打印函数时，输出的格式

// a）只输出数据
struct __zh_Print_with_none
{
};

// b）在数据之后加上空格
struct __zh_Print_with_space
{
};

// c）在数据之后加上逗号和空格
struct __zh_Print_with_delimiter
{
};

// 工作3、约束：使用默认打印函数时，打印格式能且仅能以上三种方式之一
template <typename PrintMode>
concept __is_print_mode = (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_none>) ||
						  (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_space>) ||
						  (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_delimiter>);

// 工作4、对于基础数据类型，采用如下的输出方式，称此方式为 基础输出方式
struct __zh_Default_print_with_one_data_function
{
public:

	template <__basic_msg_type MsgType, __is_print_mode PrintMode = __zh_Print_with_delimiter>
	constexpr void operator()(const MsgType& msg, PrintMode mode = {}) const noexcept
	{
		if constexpr (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_delimiter>)
		{
			_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}, ", msg);
		}
		else if constexpr (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_space>)
		{
			_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{} ", msg);
		}
		else if constexpr (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_none>)
		{
			_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}", msg);
		}
	}

	template <__is_print_mode PrintMode = __zh_Print_with_delimiter>
	constexpr void operator()(const wchar_t* msg, PrintMode mode = {}) const noexcept
	{
		_STD ios::sync_with_stdio(true);
		_STD locale::global(_STD locale(""));

		if constexpr (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_delimiter>)
		{
			_STD format_to(_STD ostreambuf_iterator<wchar_t> { _STD wcout }, L"{}, ", msg);
		}
		else if constexpr (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_space>)
		{
			_STD format_to(_STD ostreambuf_iterator<wchar_t> { _STD wcout }, L"{} ", msg);
		}
		else if constexpr (_STD is_same_v<_STD remove_cvref_t<PrintMode>, __zh_Print_with_none>)
		{
			_STD format_to(_STD ostreambuf_iterator<wchar_t> { _STD wcout }, L"{}", msg);
		}
	}
};

// 工作5、定义别名：是为了方便使用者快速定义投影函数的同时，不改变默认的打印方式
using default_print = __zh_Default_print_with_one_data_function;

// 以上是准备工作

/*-----------------------------------------------------------------------------------------------------*/

// 此处实现 print() （ 此函数不属于 STL ，只是基于 C++20 标准封装的 多功能打印函数 print() ）
struct __Format_print_function: private __Not_quite_object
{
private:

	// 当 [first, last) 区间中元素类型是 char 或 wchar_t 时的特化版本
	template <_STD input_iterator InputIterator,
			  typename PrintFunction = __zh_Default_print_with_one_data_function,
			  typename Projection	 = _STD identity>
		requires(_STD is_same_v<_STD remove_cvref_t<typename _STD iter_value_t<InputIterator>>, char> ||
				 _STD is_same_v<_STD remove_cvref_t<typename _STD iter_value_t<InputIterator>>, wchar_t>)
	static constexpr void __print_with_char_or_wchar(InputIterator first,
													 InputIterator last,
													 PrintFunction pfun,
													 Projection	   proj) noexcept(true)
	{
		using value_type = typename _STD iter_value_t<InputIterator>;

		if constexpr (_STD is_same_v<_STD remove_cvref_t<value_type>,
									 char>) // 如果是字符类型，转为 string ，调用 fputs 输出
		{
			if constexpr ((_STD is_same_v<_STD remove_cvref_t<Projection>, _STD identity>)&&(
							  noexcept(_STD string(first, last)))) // 如果没有自定义的投影函数 proj ，直接输出
			{
				fputs(_STD string(first, last).c_str(), stdout);
			}
			else // 如果有自定义的投影函数 proj ，先投影，再输出
			{
				// 使用 std::cout 作为标准输出目的地
				auto standard_output_destination_with_char { _STD ostreambuf_iterator<char> { _STD cout } };

				for (; (first != last) && (*first != '\0'); ++first)
				{
					_STD format_to(standard_output_destination_with_char, "{}", _STD invoke(proj, *first));
				}
			}

			return;
		}
		else if constexpr (_STD is_same_v<
							   _STD remove_cvref_t<value_type>,
							   wchar_t>) // 如果是宽字符类型且设置模式不失败，转为 wstring ，调用 std::wcout 输出
		{
			_STD ios::sync_with_stdio(true);
			_STD locale::global(_STD locale(""));

			if constexpr ((_STD is_same_v<_STD remove_cvref_t<Projection>, _STD identity>)&&(
							  noexcept(_STD wstring(first, last)))) // 如果没有自定义的投影函数 proj ，直接输出
			{
				_STD wcout << _STD move(_STD wstring(first, last));
			}
			else // 如果有自定义的投影函数 proj ，先投影，再输出
			{
				// 使用 std::wcout 作为标准输出目的地
				auto standard_output_destination_with_wchar_t { _STD ostreambuf_iterator<wchar_t> { _STD wcout } };

				for (; (first != last) && (*first != L'\0'); ++first)
				{
					_STD format_to(standard_output_destination_with_wchar_t, L"{}", _STD invoke(proj, *first));
				}
			}

			return;
		}
	}

	// 使用自定义打印函数 pfun 和投影函数 proj ，格式化输出 [first, last) 区间内的所有元素
	template <_STD input_iterator InputIterator,
			  typename PrintFunction = __zh_Default_print_with_one_data_function,
			  typename Projection	 = _STD identity>
	static constexpr void
		__print_with_format_iter(InputIterator first, InputIterator last, PrintFunction pfun, Projection proj) noexcept(
			(noexcept(_STD invoke(pfun, _STD invoke(proj, *first)))) ||
			(noexcept(_STD invoke(pfun, _STD invoke(proj, *first), __zh_Print_with_delimiter {}))))
	{
		if constexpr (_STD is_same_v<_STD remove_cvref_t<PrintFunction>, __zh_Default_print_with_one_data_function>)
		{
			fputs("[ ", stdout);

			for (; first != (last - 1); ++first)
			{
				_STD invoke(pfun, _STD invoke(proj, *first), __zh_Print_with_delimiter {});
			}

			_STD invoke(pfun, _STD invoke(proj, *first), __zh_Print_with_none {});

			fputs(" ]", stdout);
		}
		else
		{
			for (; first != last; ++first)
			{
				_STD invoke(pfun, _STD invoke(proj, *first));
			}
		}
	}

	// 顶级输出语句之一。使用默认打印函数 pfun ，按预定义打印方式顺次输出参数包 args 中的所有参数
	template <typename PrintFunction, __is_print_mode PrintMode, __basic_msg_type Arg, __basic_msg_type... Args>
		requires(_STD is_same_v<_STD remove_cvref_t<PrintFunction>, __zh_Default_print_with_one_data_function>)
	static constexpr void __print_with_args(PrintFunction pfun, PrintMode mode, Arg&& arg, Args&&... args) noexcept
	{
		if constexpr (sizeof...(args) == 0) // 参包为空，即只有一个待输出的参数，直接输出，不加任何修饰
		{
			if constexpr (_STD is_null_pointer_v<Arg>)
			{
				_STD invoke(pfun, "nullptr", __zh_Print_with_none {});
			}
			else
			{
				_STD invoke(pfun, _STD forward<Arg&&>(arg), __zh_Print_with_none {});
			}

			return;
		}
		else // 参包不为空，即有多个待输出的参数，顺次输出，一般情况下是在每个参数之间用逗号和空格分隔
		{
			if constexpr (_STD is_null_pointer_v<Arg>)
			{
				_STD invoke(pfun, "nullptr", mode);
			}
			else
			{
				_STD invoke(pfun, _STD forward<Arg&&>(arg), mode);
			}

			__print_with_args(pfun, mode, _STD forward<Args&&>(args)...);

			return;
		}
	}

	// 顶级输出语句之二。使用迭代器 first 和 last ，顺次输出 [first, last) 区间内的所有元素
	template <_STD input_iterator InputIterator,
			  typename PrintFunction = __zh_Default_print_with_one_data_function,
			  typename Projection	 = _STD identity>
	static constexpr void
		__print_with_iter(InputIterator first, InputIterator last, PrintFunction pfun, Projection proj) noexcept

	{
		using difference_type = typename _STD iter_difference_t<InputIterator>;
		using value_type	  = typename _STD	   iter_value_t<InputIterator>;

		// 如果是字符类型的指针，调用 __print_with_char_or_wchar() 。特别注意，字符类型的判断必须放在前面，否则会被误判为算术类型
		if constexpr (((_STD is_same_v<_STD remove_cvref_t<value_type>, char>) ||
					   (_STD is_same_v<_STD remove_cvref_t<value_type>, wchar_t>)) &&
					  ((_STD is_pointer_v<InputIterator>) || (_STD is_array_v<InputIterator>)))
		{
			if (0 < (last - first)) // 如果 first，last 指向同一个字符串，输出这个字符串的信息
			{
				__print_with_char_or_wchar(_STD move(first), _STD move(last), pfun, proj);
			}
			else // 如果 first，last 指向不同的字符串，使用格式化输出
			{
				__print_with_basic_msg(_STD move(first), _STD move(last));
			}

			return;
		}
		else if constexpr (
			(_STD is_compound_v<value_type>) ||
			(!(_STD is_same_v<
				_STD remove_cvref_t<PrintFunction>,
				__zh_Default_print_with_one_data_function>))) // 如果是复合类型 或 已有自定义的打印函数 pfun ，则使用自定义打印函数 pfun ，顺次输出所有元素
		{
			if constexpr (_STD is_pointer_v<value_type>)
			{
				using value_value_type = typename _STD iter_value_t<value_type>;

				if (_STD is_fundamental_v<value_value_type>)
				{
					__print_with_format_iter(_STD move(first), _STD move(last), pfun, proj);

					return;
				}
			}
			else
			{
				__print_with_format_iter(_STD move(first), _STD move(last), pfun, proj);

				return;
			}
		}
		else if constexpr (_STD is_arithmetic_v<value_type>) // 如果是算术类型，适当美化后输出
		{
			// 判断经过投影的数据是 整数类型 还是 浮点类型（判断仅做一次）
			constexpr bool is_float_type { _STD is_floating_point_v<decltype(_STD invoke(proj, *first))> };

			// 使用 std::cout 作为标准输出目的地
			auto standard_output_destination_with_char { _STD ostreambuf_iterator<char> { _STD cout } };

			fputs("[ ", stdout);

			for (; first != (last - 1); ++first)
			{
				if constexpr (is_float_type)
				{
					_STD format_to(standard_output_destination_with_char, "{:.5f}, ", _STD invoke(proj, *first));
				}
				else
				{
					_STD format_to(standard_output_destination_with_char, "{}, ", _STD invoke(proj, *first));
				}
			}

			if constexpr (is_float_type)
			{
				_STD format_to(standard_output_destination_with_char, "{:.5f} ]", _STD invoke(proj, *first));
			}
			else
			{
				_STD format_to(standard_output_destination_with_char, "{} ]", _STD invoke(proj, *first));
			}

			return;
		}
	}

	// 顶级输出语句之三。首先尝试使用参包 args 格式化 msg ，若成功，则输出格式化后的字符串，否则顺次输出 msg 和参数包 args 中的所有参数
	template <__basic_msg_type MsgType, __basic_msg_type... Args>
	static constexpr void __print_with_basic_msg(MsgType&& msg, Args&&... args) noexcept
	{
		if constexpr (_STD is_fundamental_v<MsgType>) // 如果是基本类型的组合，直接输出
		{
			__print_with_args(__zh_Default_print_with_one_data_function {},
							  __zh_Print_with_delimiter {},
							  _STD forward<MsgType&&>(msg),
							  _STD forward<Args&&>(args)...);

			return;
		}
		else // 否则，尝试格式化输出
		{
			// 使用 std::cout 作为标准输出目的地
			auto standard_output_destination_with_char { _STD ostreambuf_iterator<char> { _STD cout } };

			if (const auto& len_for_args { sizeof...(args) }; len_for_args == 0) // 如果格式化参包 args 为空，直接输出
			{
				_STD format_to(standard_output_destination_with_char, "{}", msg);
			}
			else
			{
				_STD string fmt_msg(msg);									  // 用于格式化的字符串
				_STD string new_fmt_msg { _STD move(
					_STD vformat(fmt_msg, _STD make_format_args(args...))) }; // 格式化后的字符串

				if (fmt_msg != new_fmt_msg) // 如果格式化成功，输出格式化后的字符串
				{
					_STD format_to(standard_output_destination_with_char, "{}", new_fmt_msg);
				}
				else // 否则，顺次输出 msg 和参数包 args 中的所有参数
				{
					__print_with_args(__zh_Default_print_with_one_data_function {},
									  __zh_Print_with_delimiter {},
									  _STD forward<MsgType&&>(msg),
									  _STD forward<Args&&>(args)...);
				}
			}

			return;
		}
	}

public:

	using __Not_quite_object::__Not_quite_object;

	// 0.1、输出空行
	void operator()(void) const noexcept(true)
	{
		fputs("\n", stdout);
	}

	// 1、针对 迭代器 的一般泛化
	template <typename InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  typename PrintFunction = __zh_Default_print_with_one_data_function,
			  typename Projection	 = _STD identity>
		requires(requires(InputIterator iterator, PrintFunction pfun, Projection proj) {
			requires((_STD input_iterator<InputIterator>) || (_STD is_pointer_v<InputIterator>));

			{
				_STD invoke(__check_function(pfun), _STD invoke(__check_function(proj), *iterator))
			} noexcept -> _STD same_as<void>;
		})
	constexpr void operator()(InputIterator first, Sentinel last, PrintFunction pfun = {}, Projection proj = {}) const
		noexcept(noexcept(
			__print_with_iter(_STD move(first), _STD move(last), __check_function(pfun), __check_function(proj))))
	{
		auto check_first = __unwrap_iterator<Sentinel>(_STD move(first));
		auto check_last	 = __get_last_iterator_with_unwrapped<InputIterator, Sentinel>(check_first, _STD move(last));

		__print_with_iter(_STD move(check_first),
						  _STD move(check_last),
						  __check_function(pfun),
						  __check_function(proj));
	}

	// 2、针对 容器 的特化
	template <_RANGES input_range Range,
			  typename PrintFunction = __zh_Default_print_with_one_data_function,
			  typename Projection	 = _STD identity>
		requires(requires(typename _RANGES range_value_t<Range> val, PrintFunction pfun, Projection proj) {
			{
				_STD invoke(__check_function(pfun), _STD invoke(__check_function(proj), val))
			} noexcept -> _STD same_as<void>;
		})
	constexpr void operator()(Range&& rng, PrintFunction pfun = {}, Projection proj = {}) const
		noexcept(noexcept((*this)(_RANGES begin(rng), _RANGES end(rng), pfun, proj)))
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pfun, proj);
	}

	// 3、针对 format() 格式的 一般泛化
	template <__basic_msg_type MsgType, __basic_msg_type... Args>
	constexpr void operator()(MsgType msg, Args... args) const
		noexcept(noexcept(__print_with_basic_msg(_STD forward<MsgType&&>(msg), _STD forward<Args&&>(args)...)))
	{
		static_assert((sizeof...(args) < (__max_msg_args_constant<uint64_t>)),
					  "There are too many parameters, please consider printing in an STL container.");

		__print_with_basic_msg(_STD forward<MsgType&&>(msg), _STD forward<Args&&>(args)...);
	}
};

constexpr inline __Format_print_function print { __Not_quite_object::__Construct_tag {} };

/*-----------------------------------------------------------------------------------------------------*/

// 此处实现 println<...>(...)
struct __Format_println_function: private __Not_quite_object
{
public:

	using __Not_quite_object::__Not_quite_object;

	void operator()(void) const noexcept(true)
	{
		fputs("\n", stdout);
	}

	template <typename InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  typename PrintFunction = __zh_Default_print_with_one_data_function,
			  typename Projection	 = _STD identity>
		requires(requires(InputIterator iterator, PrintFunction pfun, Projection proj) {
			requires((_STD input_iterator<InputIterator>) || (_STD is_pointer_v<InputIterator>));

			{
				_STD invoke(__check_function(pfun), _STD invoke(__check_function(proj), *iterator))
			} noexcept -> _STD same_as<void>;
		})
	constexpr void operator()(InputIterator first, Sentinel last, PrintFunction pfun = {}, Projection proj = {}) const
		noexcept(noexcept(print(_STD move(first), _STD move(last), pfun, proj)))
	{
		print(_STD move(first), _STD move(last), pfun, proj);
		fputs("\n", stdout);
	}

	template <_RANGES input_range Range,
			  typename PrintFunction = __zh_Default_print_with_one_data_function,
			  typename Projection	 = _STD identity>
		requires(requires(typename _RANGES range_value_t<Range> value, PrintFunction pfun, Projection proj) {
			{
				_STD invoke(__check_function(pfun), _STD invoke(__check_function(proj), value))
			} noexcept -> _STD same_as<void>;
		})
	constexpr void operator()(Range&& rng, PrintFunction pfun = {}, Projection proj = {}) const
		noexcept(noexcept((*this)(_RANGES begin(rng), _RANGES end(rng), pfun, proj)))
	{
		(*this)(_RANGES begin(rng), _RANGES end(rng), pfun, proj);
	}

	template <__basic_msg_type MsgType, __basic_msg_type... Args>
	constexpr void operator()(MsgType msg, Args... args) const
		noexcept(noexcept(print(_STD forward<MsgType&&>(msg), _STD forward<Args&&>(args)...)))
	{
		static_assert((sizeof...(args) < (__max_msg_args_constant<uint64_t>)),
					  "There are too many parameters, please consider printing in an STL container.");

		print(_STD forward<MsgType&&>(msg), _STD forward<Args&&>(args)...);
		fputs("\n", stdout);
	}
};

constexpr inline __Format_println_function println { __Not_quite_object::__Construct_tag {} };

/*-----------------------------------------------------------------------------------------------------*/



struct __Take_msg
{
	const char* info;

	constexpr explicit(false) __Take_msg(const char* str) noexcept: info(str)
	{
	}
};

template <__Take_msg msg>
consteval _STD string operator""_f() noexcept
{
	return [=]<typename... Type>(Type... Args) constexpr
	{
		return _STD format(msg.info, _STD forward<Type&&>(Args)...);
	};
}

template <typename Type>
	requires(_STD is_pointer_v<Type>)
inline _STD string ads(const Type p) noexcept
{
	return _STD format("{}", p);
}

__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
