#pragma once

#include "00_basicFile.h"


#ifdef __HAS_CPP20

__BEGIN_NAMESPACE_ZHANG

// 此处实现 pair<...>(...)
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
	constexpr bool operator==(const pair<FirstType, SecondType>& value) const noexcept
	{
		return __invoke(_RANGES equal_to {}, value.first, this->first) &&
			   __invoke(_RANGES equal_to {}, value.second, this->second);
	}

	// 此处实现 make_pair(...)
	template <typename FirstType, typename SecondType>
	static constexpr pair<FirstType, SecondType> make_pair(const FirstType& value1, const SecondType& value2) noexcept
	{
		return pair<FirstType, SecondType>(value1, value2);
	}
};

// pair 推断指引
template <typename FirstType, typename SecondType>
pair(FirstType, SecondType) -> pair<FirstType, SecondType>;

/*-----------------------------------------------------------------------------------------------------*/

// 此处实现 print<...>(...)

// 以下是准备工作

// 工作1、定义：使用默认打印函数时，输出的格式

// a）只输出数据
struct __print_with_none
{
};

// b）在数据之后加上空格
struct __print_with_space: public __print_with_none
{
};

// c）在数据之后加上逗号和空格
struct __print_with_delimiter: public __print_with_space
{
};

// 工作2、约束：打印方式必须是以上三种方式之一
template <typename PrintMode>
concept __is_print_mode =
	(_STD is_same_v<PrintMode, __print_with_none>) || (_STD is_same_v<PrintMode, __print_with_space>) ||
	(_STD is_same_v<PrintMode, __print_with_delimiter>);

// 工作3、对于基础数据类型，采用如下的输出方式，称此方式为 基础输出方式
struct __print_with_basic_approach_function
{
public:

	template <__basic_msg_type T, __is_print_mode PrintMode = __print_with_delimiter>
	constexpr void operator()(const T& msg, PrintMode mode = {}) const noexcept
	{
		if constexpr (_STD is_same_v<PrintMode, __print_with_delimiter>)
		{
			_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}, ", msg);
		}
		else if constexpr (_STD is_same_v<PrintMode, __print_with_space>)
		{
			_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{} ", msg);
		}
		else if constexpr (_STD is_same_v<PrintMode, __print_with_none>)
		{
			_STD format_to(_STD ostreambuf_iterator<char> { _STD cout }, "{}", msg);
		}
	}

	template <__is_print_mode PrintMode = __print_with_delimiter>
	void operator()(const wchar_t* msg, PrintMode mode = {}) const noexcept
	{
		_STD ios::sync_with_stdio(true);
		_STD locale::global(_STD locale(""));

		if constexpr (_STD is_same_v<PrintMode, __print_with_delimiter>)
		{
			_STD format_to(_STD ostreambuf_iterator<wchar_t> { _STD wcout }, L"{}, ", msg);
		}
		else if constexpr (_STD is_same_v<PrintMode, __print_with_space>)
		{
			_STD format_to(_STD ostreambuf_iterator<wchar_t> { _STD wcout }, L"{} ", msg);
		}
		else if constexpr (_STD is_same_v<PrintMode, __print_with_none>)
		{
			_STD format_to(_STD ostreambuf_iterator<wchar_t> { _STD wcout }, L"{}", msg);
		}
	}
};

constexpr inline __print_with_basic_approach_function __print_with_basic_approach {};

// 工作4、定义别名：是为了方便使用者快速定义投影函数的同时，不改变默认的打印方式
using default_print = __print_with_basic_approach_function;

// 以上是准备工作

/*-----------------------------------------------------------------------------------------------------*/

// 此处实现 print() （ 此函数不属于 STL ，只是基于 C++20 标准封装的 多功能打印函数 print() ）
struct __print_function: private __not_quite_object
{
private:

	// 当 [pointer1, last) 区间中元素类型是 char 或 wchar_t 时的特化版本
	template <__is_input_iterator InputIterator,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires(_STD is_same_v<__value_type_for_iter<InputIterator>, char> ||
				 _STD is_same_v<__value_type_for_iter<InputIterator>, wchar_t>)
	static constexpr void
		__print_with_char_or_wchar(InputIterator first, InputIterator last, Function fun, Projection proj) noexcept
	{
		using value_type = __value_type_for_iter<InputIterator>;

		if constexpr (_STD is_same_v<value_type, char>) // 如果是字符类型，转为 string ，调用 fputs 输出
		{
			if constexpr ((_STD is_same_v<Projection, _STD identity>)&&(
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
					_STD format_to(standard_output_destination_with_char, "{}", __invoke(proj, *first));
				}
			}

			return;
		}
		else if constexpr (_STD is_same_v<
							   value_type,
							   wchar_t>) // 如果是宽字符类型且设置模式不失败，转为 wstring ，调用 std::wcout 输出
		{
			_STD ios::sync_with_stdio(true);
			_STD locale::global(_STD locale(""));

			if constexpr ((_STD is_same_v<Projection, _STD identity>)&&(
							  noexcept(_STD wstring(first, last)))) // 如果没有自定义的投影函数 proj ，直接输出
			{
				_STD wcout << __move(_STD wstring(first, last));
			}
			else // 如果有自定义的投影函数 proj ，先投影，再输出
			{
				// 使用 std::wcout 作为标准输出目的地
				auto standard_output_destination_with_wchar_t { _STD ostreambuf_iterator<wchar_t> { _STD wcout } };

				for (; (first != last) && (*first != L'\0'); ++first)
				{
					_STD format_to(standard_output_destination_with_wchar_t, L"{}", __invoke(proj, *first));
				}
			}

			return;
		}
	}

	// 使用自定义打印函数 fun 和投影函数 proj ，格式化输出 [first, last) 区间内的所有元素
	template <__is_input_iterator InputIterator,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
	static constexpr void
		__print_with_format_iter(InputIterator first, InputIterator last, Function fun, Projection proj) noexcept
	{
		if constexpr (_STD is_same_v<Function, __print_with_basic_approach_function>)
		{
			fputs("[ ", stdout);

			for (; first != (last - 1); ++first)
			{
				__invoke(fun, __invoke(proj, *first), __print_with_delimiter {});
			}

			__invoke(fun, __invoke(proj, *first), __print_with_none {});

			fputs(" ]", stdout);
		}
		else
		{
			for (; first != last; ++first)
			{
				__invoke(fun, __invoke(proj, *first));
			}
		}
	}

	// 顶级输出语句之一。使用默认打印函数 fun ，按预定义打印方式顺次输出参数包 args 中的所有参数
	template <typename Function, __is_print_mode PrintMode, typename... Args>
		requires(_STD is_same_v<Function, __print_with_basic_approach_function>)
	static constexpr void __print_with_args(Function fun, PrintMode mode, Args&&... args) noexcept
	{
		(__invoke(fun, _STD forward<Args&&>(args), mode), ...);
	}

	// 顶级输出语句之二。使用迭代器 first 和 last ，顺次输出 [first, last) 区间内的所有元素
	template <__is_iter_or_array InputIterator,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
	static constexpr void
		__print_with_iter(InputIterator first, InputIterator last, Function fun, Projection proj) noexcept
	{
		using difference_type = __difference_type_for_iter<InputIterator>;
		using value_type	  = __value_type_for_iter<InputIterator>;

		// 如果是字符类型的指针，调用 __print_with_char_or_wchar() 。特别注意，字符类型的判断必须放在前面，否则会被误判为算术类型
		if constexpr (((_STD is_same_v<value_type, char>) || (_STD is_same_v<value_type, wchar_t>)) &&
					  ((_STD is_pointer_v<InputIterator>) || (_STD is_array_v<InputIterator>)))
		{
			if (0 < (last - first)) // 如果 first，last 指向同一个字符串，输出这个字符串的信息
			{
				__print_with_char_or_wchar(__move(first), __move(last), fun, proj);
			}
			else // 如果 first，last 指向不同的字符串，使用格式化输出
			{
				__print_with_basic_mag(__move(first), __move(last));
			}

			return;
		}
		else if constexpr (
			(__is_compound_type<value_type>) ||
			(!(_STD is_same_v<
				Function,
				__print_with_basic_approach_function>))) // 如果是复合类型 或 已有自定义的输出函数 fun ，使用自定义打印函数 fun ，顺次输出所有元素
		{
			if constexpr (_STD is_pointer_v<value_type>)
			{
				using value_value_type = __value_type_for_iter<value_type>;

				if (__not_compound_type<value_value_type>)
				{
					__print_with_format_iter(__move(first), __move(last), fun, proj);

					return;
				}
			}
			else if constexpr (noexcept(__invoke(fun, __invoke(proj, *first))))
			{
				__print_with_format_iter(__move(first), __move(last), fun, proj);

				return;
			}


			fputs("Error! Unable to output this type of data using the currently provided Print Function. "
				  "Please try providing a new Printing Function.",
				  stdout);

			return;
		}
		else if constexpr (_STD is_arithmetic_v<value_type>) // 如果是算术类型，适当美化后输出
		{
			// 判断经过投影的数据是 整数类型 还是 浮点类型（判断仅做一次）
			constexpr bool is_float_type { _STD is_floating_point_v<decltype(__invoke(proj, *first))> };

			// 使用 std::cout 作为标准输出目的地
			auto standard_output_destination_with_char { _STD ostreambuf_iterator<char> { _STD cout } };

			fputs("[ ", stdout);

			for (; first != (last - 1); ++first)
			{
				if constexpr (is_float_type) // 如果是浮点类型，保留 3 位小数
				{
					_STD format_to(standard_output_destination_with_char, "{:.5f}, ", __invoke(proj, *first));
				}
				else // 否则，按默认格式输出
				{
					_STD format_to(standard_output_destination_with_char, "{}, ", __invoke(proj, *first));
				}
			}

			if constexpr (is_float_type)
			{
				_STD format_to(standard_output_destination_with_char, "{:.5f} ]", __invoke(proj, *first));
			}
			else
			{
				_STD format_to(standard_output_destination_with_char, "{} ]", __invoke(proj, *first));
			}

			return;
		}
	}

	// 顶级输出语句之三。首先尝试使用参包 args 格式化 msg ，若成功，则输出格式化后的字符串，否则顺次输出 T 和参数包 args 中的所有参数
	template <__basic_msg_type T, __basic_msg_type... Args>
	static constexpr void __print_with_basic_mag(T&& msg, Args&&... args) noexcept
	{
		if constexpr (__not_compound_type<T>) // 如果是基本类型的组合，直接输出
		{
			__print_with_args(__print_with_basic_approach, __print_with_delimiter {}, msg, _STD forward<Args>(args)...);

			return;
		}
		else // 否则，尝试格式化输出
		{
			// 使用 std::wcout 作为标准输出目的地
			auto standard_output_destination_with_char { _STD ostreambuf_iterator<char> { _STD cout } };

			_STD string data_tmp(msg);

			if (const auto& len_for_args { sizeof...(args) }; len_for_args == 0) // 如果格式化参包 args 为空，直接输出
			{
				_STD format_to(standard_output_destination_with_char, "{}", msg);
			}
			else
			{
				_STD string new_fmt_msg { __move(_STD vformat(data_tmp, _STD make_format_args(args...))) };

				// 无论如何都输出第一个参数（即格式化参包之前的那一个参数）
				_STD format_to(standard_output_destination_with_char, "{}", new_fmt_msg);

				if (data_tmp == new_fmt_msg) // 如果格式化后的字符串和原字符串相同，即格式化失败
				{
					fputs(", ", stdout);

					// 顺次输出参包的所有参数
					__print_with_args(__print_with_basic_approach,
									  __print_with_delimiter {},
									  _STD forward<Args>(args)...);
				}
			}

			return;
		}
	}

public:

	using __not_quite_object::__not_quite_object;

	// 0.1、输出空行
	void operator()(void) const noexcept
	{
		fputs("\n", stdout);
	}

	// 1.1、针对指向 基础类型数据的 迭代器 的一般泛化
	template <__is_iter_or_array InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires(__not_compound_type<__value_type_for_iter<InputIterator>>) &&
				requires(InputIterator it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, *it)));
				}
	constexpr void
		operator()(InputIterator first, Sentinel last, Function fun = {}, Projection proj = {}) const noexcept
	{
		auto check_first = _RANGES _Unwrap_iter<Sentinel>(__move(first));
		auto check_last	 = _RANGES _Get_final_iterator_unwrapped<InputIterator>(check_first, __move(last));

		__print_with_iter(__move(check_first), __move(check_last), fun, proj);
	}

	// 1.2、针对指向 复合类型数据的 迭代器 的一般泛化
	template <__is_iter_or_array InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires(__is_compound_type<__value_type_for_iter<InputIterator>>) &&
				requires(InputIterator it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, *it)));
				}
	constexpr void operator()(InputIterator first, Sentinel last, Function fun, Projection proj = {}) const noexcept
	{
		auto check_first = _RANGES _Unwrap_iter<Sentinel>(__move(first));
		auto check_last	 = _RANGES _Get_final_iterator_unwrapped<InputIterator>(check_first, __move(last));

		__print_with_iter(__move(check_first), __move(check_last), fun, proj);
	}

	// 2.1、针对容纳 基础类型数据的 容器 的特化
	template <__is_input_range Range,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires((__not_compound_type<__value_type_for_range<Range>>) ||
				 ((_STD is_pointer_v<__value_type_for_range<Range>>)&&(
					 !__is_compound_type<__value_type_for_iter<__value_type_for_range<Range>>>))) &&
				requires(__value_type_for_range<Range> it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, it)));
				}
	constexpr void operator()(Range&& rng, Function fun = {}, Projection proj = {}) const noexcept(
		noexcept(__print_with_iter(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj)))
	{
		__print_with_iter(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}

	// 2.2、针对容纳 复合类型数据的 容器 的特化
	template <__is_input_range Range,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires(__is_compound_type<__value_type_for_range<Range>>) &&
				requires(__value_type_for_range<Range> it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, it)));
				}
	constexpr void operator()(Range&& rng, Function fun, Projection proj = {}) const noexcept(
		noexcept(__print_with_iter(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj)))
	{
		__print_with_iter(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
	}

	// 3.1、针对 format() 格式的 一般泛化（右值）
	template <__basic_msg_type T, __basic_msg_type... Args>
	constexpr void operator()(T&& msg, Args&&... args) const
		noexcept(noexcept(__print_with_basic_mag(__move(msg), _STD forward<Args>(args)...)))
	{
		__print_with_basic_mag(_STD forward<T&&>(msg), _STD forward<Args&&>(args)...);
	}

	// 3.2、针对 format() 格式的 一般泛化
	template <__basic_msg_type T, __basic_msg_type... Args>
	constexpr void operator()(const T& msg, Args&&... args) const
		noexcept(noexcept(__print_with_basic_mag(__move(msg), _STD forward<Args>(args)...)))
	{
		__print_with_basic_mag(_STD forward<const T&>(msg), _STD forward<Args&&>(args)...);
	}
};

constexpr inline __print_function print { __not_quite_object::__construct_tag {} };

/*-----------------------------------------------------------------------------------------------------*/

// 此处实现 println<...>(...)
struct __println_function: private __not_quite_object
{
public:

	using __not_quite_object::__not_quite_object;

	void operator()(void) const noexcept
	{
		fputs("\n", stdout);
	}

	template <__is_iter_or_array InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires(__not_compound_type<__value_type_for_iter<InputIterator>>) &&
				requires(InputIterator it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, *it)));
				}
	constexpr void
		operator()(InputIterator first, Sentinel last, Function fun = {}, Projection proj = {}) const noexcept
	{
		print(_STD forward<InputIterator>(__move(first)), _STD forward<Sentinel>(__move(last)), fun, proj);
		fputs("\n", stdout);
	}

	template <__is_iter_or_array InputIterator,
			  _STD sentinel_for<InputIterator> Sentinel,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires(__is_compound_type<__value_type_for_iter<InputIterator>>) &&
				requires(InputIterator it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, *it)));
				}
	constexpr void operator()(InputIterator first, Sentinel last, Function fun, Projection proj = {}) const noexcept
	{
		print(_STD forward<InputIterator>(__move(first)), _STD forward<Sentinel>(__move(last)), fun, proj);
		fputs("\n", stdout);
	}

	template <__is_input_range Range,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires((__not_compound_type<__value_type_for_range<Range>>) ||
				 ((_STD is_pointer_v<__value_type_for_range<Range>>)&&(
					 !__is_compound_type<__value_type_for_iter<__value_type_for_range<Range>>>))) &&
				requires(__value_type_for_range<Range> it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, it)));
				}
	constexpr void operator()(Range&& rng, Function fun = {}, Projection proj = {}) const
		noexcept(noexcept(print(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj)))
	{
		print(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
		fputs("\n", stdout);
	}

	template <__is_input_range Range,
			  class Function	  = __print_with_basic_approach_function,
			  typename Projection = _STD identity>
		requires(__is_compound_type<__value_type_for_range<Range>>) &&
				requires(__value_type_for_range<Range> it, Function fun, Projection proj) {
					noexcept(__invoke(fun, __invoke(proj, it)));
				}
	constexpr void operator()(Range&& rng, Function fun, Projection proj = {}) const
		noexcept(noexcept(print(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj)))
	{
		print(__begin_for_range_with_move(rng), __end_for_range_with_move(rng), fun, proj);
		fputs("\n", stdout);
	}

	template <__basic_msg_type T, __basic_msg_type... Args>
	constexpr void operator()(T&& msg, Args&&... args) const
		noexcept(noexcept(print(_STD forward<T&&>(msg), _STD forward<Args&&>(args)...)))
	{
		print(_STD forward<T&&>(msg), _STD forward<Args&&>(args)...);
		fputs("\n", stdout);
	}

	template <__basic_msg_type T, __basic_msg_type... Args>
	constexpr void operator()(const T& msg, Args&&... args) const
		noexcept(noexcept(print(_STD forward<const T&>(msg), _STD forward<Args>(args)...)))
	{
		print(_STD forward<const T&>(msg), _STD forward<Args&&>(args)...);
		fputs("\n", stdout);
	}
};

constexpr inline __println_function println { __not_quite_object::__construct_tag {} };

__END_NAMESPACE_ZHANG

#endif // __HAS_CPP20
