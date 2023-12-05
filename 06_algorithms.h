#pragma once

#include "00_basicFile.h"

/*
* 如果没有实现自定义的容器, 则:
*
*	重要！所有 “迭代器类型参数（例如 input_iterator_tag ）” 都要使用命名空间 ::std:: 中定义的迭代器类型！
*	重要！所有 “迭代器类型参数（例如 input_iterator_tag ）” 都要使用命名空间 ::std:: 中定义的迭代器类型！
*	重要！所有 “迭代器类型参数（例如 input_iterator_tag ）” 都要使用命名空间 ::std:: 中定义的迭代器类型！
*
*		原因如下：
*	目前使用的容器是 MSVC STL 中定义在命名空间 ::std:: 内的，尚未自己实现。这导致 __ZH_ITER__ 中萃取函数 iterator_category() 的返回值都是命名空间 ::std:: 中定义的迭代器类型（因为萃取的就是 ::std:: 中的 iterator , 那么返回值当然也是 ::std:: 中定义的类型）。如果使用自定义的迭代器类型，则会有以下报错:
*	“没有适当的转换函数，使类型 ‘ std::xxx_iterator_tag ’ 转换到类型 ‘ zhang::iterator::namespace_iterator::xxx_iterator_tag ’ ”
*
* 如果实现了自定义的容器，那么该怎么写就怎么写
*/



// 此处实现若干纯算法函数
namespace zhang::algorithms
{
	// 预定义一些用于 简写 和 标志识别 的宏
#ifndef __ZH_NAMESPACE__

	#ifndef _STD
		#define _STD ::std::
	#endif // !_STD

	#define __ZH_NAMESPACE__ ::zhang::
	#define __ZH_ITER__		 ::zhang::iterator::namespace_iterator::
	#define __ZH_PAIR__		 ::zhang::without_book::namespace_pair::
	#define __ZH_HEAP__		 ::zhang::sequence_containers::namespace_heap::

#endif // !__ZH_NAMESPACE__

#ifdef __HAS_CPP20

	template <typename T>
	concept __is_iterator = requires(T p) { *p; };

	template <typename T>
	concept __is_container = requires(T p) {
		typename T::value_type;
		p.begin();
		p.end();
	};

	#ifdef _MSC_VER
	template <class Function>
	constexpr auto __check_fun(Function& fun) noexcept
	{
		constexpr bool __pass_by_value = _STD conjunction_v<_STD bool_constant<sizeof(Function) <= sizeof(void*)>,
															_STD is_trivially_copy_constructible<Function>,
															_STD is_trivially_destructible<Function>>;
		if constexpr (__pass_by_value)
		{
			return fun;
		}
		else
		{
			return _STD _Ref_fn<Function> { fun }; // pass functor by "reference"
		}
	}
	#endif

	#ifdef __MINGW32__
	template <typename Function>
	constexor auto __check_fun(Function& fun)
	{
		return __gnu_cxx::__ops::__iter_comp_iter(fun);
	}
	#endif

	#ifdef __GNUC__

		// TODO:
	#endif

#endif // __HAS_CPP20

	// 此处实现 copy()
	namespace namespace_copy
	{
		/* function __copy_with_random_access_iter() -- 辅助函数 */
		template <typename RandomAccessIterator, typename OutputIterator, typename Distance>
		inline OutputIterator __copy_with_random_access_iter(RandomAccessIterator first,
															 RandomAccessIterator last,
															 OutputIterator		  result,
															 Distance*)
		{
			for (Distance i = last - first; i > 0; --i, ++result, ++first) // 以 i 决定循环的次数 -- 速度快
			{
				*result = *first;
			}

			return result;
		}

		/*-----------------------------------------------------------------------------------------*/



		// 如下两个函数服务于仿函数 __copy_dispatch() 的两个 偏特化版本

		/* function __copy_with_two_pointer() -- 辅助函数 */
		template <typename T> // 以下版本适用于 “指针所指之对象，具备 trivial assignment operator ”
		inline T* __copy_with_two_pointer(const T* first, const T* last, T* result, __ZH_ITER__ __true_type)
		{
			__C_FUNCTION__ memmove(result, first, sizeof(T) * (last - first));

			return result + (last - first);
		}

		template <typename T> // 以下版本适用于 “指针所指之对象，具备 non-trivial assignment operator ”
		inline T* __copy_with_two_pointer(const T* first, const T* last, T* result, __ZH_ITER__ __false_type)
		{
			return namespace_copy::__copy_with_random_access_iter(first, last, result, __cove_type(0, ptrdiff_t*));
		}

		/*-----------------------------------------------------------------------------------------*/



		// 如下两个函数服务于仿函数 __copy_dispatch() 的 完全泛化版本

		/* function __copy_difference_with_iter_tag() -- 辅助函数 */
		template <typename InputIterator, typename OutputIterator> // InputIterator 版本
		inline OutputIterator __copy_difference_with_iter_tag(InputIterator	 first,
															  InputIterator	 last,
															  OutputIterator result,
															  __ZH_ITER__	 input_iterator_tag)
		{
			for (; first != last; ++result, ++first) // 以迭代器相同与否，决定循环是否继续 -- 速度慢
			{
				*result = *first;					 // assignment operator
			}
		}

		/* function __copy_difference_with_iter_tag() -- 辅助函数 */
		template <typename RandomAccessIterator, typename OutputIterator> // RandomAccessIterator 版本
		inline OutputIterator __copy_difference_with_iter_tag(RandomAccessIterator first,
															  RandomAccessIterator last,
															  OutputIterator	   result,
															  __ZH_ITER__		   random_access_iterator_tag)
		{
			// 又划分出一个函数，为的是其他地方也能用到
			return namespace_copy::__copy_with_random_access_iter(first,
																  last,
																  result,
																  __ZH_ITER__ distance_type(first));
		}

		/*-----------------------------------------------------------------------------------------*/



		// 此仿函数直接服务于 copy()，它分为一个 完全泛化版本 和两个 偏特化版本

		/* function copy() -- 辅助函数 */
		template <typename InputIterator, typename OutputIterator> // 完全泛化版本
		struct __copy_dispatch
		{
			OutputIterator operator()(InputIterator first, InputIterator last, OutputIterator result)
			{
				return namespace_copy::__copy_difference_with_iter_tag(
					first,
					last,
					result,
					__ZH_ITER__ iterator_category(
						first)); // 此处兵分两路：根据迭代器种类的不同，调用不同的 __copy_difference_with_iter_tag() ，为的是不同种类的迭代器所使用的循环条件不同，有快慢之分
			}
		};

		/* function copy() -- 辅助函数 */
		template <typename T> // 偏特化版本1：两个参数都是 T* 指针形式
		struct __copy_dispatch<T*, T*>
		{
			T* operator()(T* first, T* last, T* result)
			{
				using t = typename __ZH_ITER__ __type_traits<T>::has_trivial_assignment_operator;
				return namespace_copy::__copy_with_two_pointer(first, last, result, t());
			}
		};

		/* function copy() -- 辅助函数 */
		template <typename T> // 偏特化版本2：第一个参数是 const T* 指针形式，第二个参数是 T* 指针形式
		struct __copy_dispatch<const T*, T*>
		{
			T* operator()(const T* first, const T* last, T* result)
			{
				using t = typename __ZH_ITER__ __type_traits<T>::has_trivial_assignment_operator;
				return namespace_copy::__copy_with_two_pointer(first, last, result, t());
			}
		};

		/*-----------------------------------------------------------------------------------------*/



		// 以下三个函数是 copy() 的对外接口

		/* function copy() 重载1 */
		inline char* copy(const char* first,
						  const char* last,
						  char* result) // 针对原生指针(可视为一种特殊的迭代器) const char* ，机型内存直接拷贝操作
		{
			__C_FUNCTION__ memmove(result, first, last - first);

			return result + (last - first);
		}

		/* function copy() 重载2 */
		inline wchar_t*
			copy(const wchar_t* first,
				 const wchar_t* last,
				 wchar_t* result) // 针对原生指针（可视为一种特殊的迭代器）const wchar_t* ，执行内存直接拷贝操作
		{
			__C_FUNCTION__ memmove(result, first, sizeof(wchar_t) * (last - first));

			return result + (last - first);
		}

		/* function copy() 一般泛型 */
		template <typename InputIterator, typename OutputIterator> // 完全泛化版本
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator>)
#endif // __HAS_CPP20
		inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
		{
			return namespace_copy::__copy_dispatch<InputIterator, OutputIterator>()(first, last, result);
		}
	} // namespace namespace_copy

	// 此处实现若干简单函数
	namespace namespace_function
	{
		/* function find() 标准版 */
		template <typename InputIterator, typename T>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator>)
#endif // __HAS_CPP20
		_NODISCARD inline InputIterator find(InputIterator first, InputIterator last, const T& value) noexcept
		{
			while ((first != last) && (*first != value))
			{
				++first;
			}

			return first;
		}

		/* function find() for 容器 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T>
			requires(algorithms::__is_container<Container>)
		_NODISCARD inline auto find(const Container& con, const T& value) noexcept
		{
			return namespace_function::find(__begin_for_container(con), __end_for_container(con), value);
		}
#endif // __HAS_CPP20


		/* function find_if() 标准版 */
		template <typename InputIterator, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator>)
#endif // __HAS_CPP20
		_NODISCARD inline InputIterator find_if(InputIterator first, InputIterator last, Function fun) noexcept
		{
#ifdef __HAS_CPP20
			fun = algorithms::__check_fun(fun);
#endif // __HAS_CPP20

			while ((first != last) && !(fun(*first)))
			{
				++first;
			}

			return first;
		}

		/* function find_if() for 容器 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename Function>
			requires(algorithms::__is_container<Container>)
		_NODISCARD inline auto find_if(const Container& con, Function fun) noexcept
		{
			return namespace_function::find_if(__begin_for_container(con), __end_for_container(con), fun);
		}
#endif // __HAS_CPP20

		/* function swap() 标准版 */
		template <typename T>
		inline void swap(T& a, T& b) noexcept
		{
			T tmp = a;
			a	  = b;
			b	  = tmp;
		}

		/* function iter_swap() -- 辅助函数 */
		template <typename ForwardIterator1, typename ForwardIterator2, typename T>
		inline void __iter_swap(ForwardIterator1 a, ForwardIterator2 b, T*) noexcept
		{
			T tmp = *a;
			*a	  = *b;
			*b	  = tmp;
		}

		/* function iter_swap() 标准版 */
		template <typename ForwardIterator1, typename ForwardIterator2>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator1> && algorithms::__is_iterator<ForwardIterator2>)
#endif // __HAS_CPP20
		inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b) noexcept
		{
			namespace_function::__iter_swap(a, b, __ZH_ITER__ value_type(a));
		}

		/* function for_each() 标准版 */
		template <typename InputIterator, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator>)
#endif // __HAS_CPP20
		inline Function for_each(InputIterator first, InputIterator last, Function fun) noexcept
		{
#ifdef __HAS_CPP20
			fun = algorithms::__check_fun(fun);
#endif // __HAS_CPP20

			for (; first != last; ++first)
			{
				fun(*first);
			}

			return fun;
		}

		/* function for_each() for 容器 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename Function>
			requires(algorithms::__is_container<Container>)
		inline Function for_each(const Container& con, Function fun) noexcept
		{
			return namespace_function::for_each(__begin_for_container(con), __end_for_container(con), fun);
		}
#endif // __HAS_CPP20

		/* function equal() for 仿函数 强化版 */
		template <typename InputIterator1, typename InputIterator2, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator1> && algorithms::__is_iterator<InputIterator2>)
#endif // __HAS_CPP20
		_NODISCARD inline bool
			equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, Function fun) noexcept
		{
#ifdef __HAS_CPP20
			fun = algorithms::__check_fun(fun);
#endif													// __HAS_CPP20

			for (; first1 != last1; ++first1, ++first2) // 如果序列 1 的元素数量多于序列 2 的元素数量, 那就糟糕了
			{
				if (fun(*first1, *first2))
				{
					return false;
				}
			}

			return true;
		}

		/* function equal() 标准版 */
		template <typename InputIterator1, typename InputIterator2>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator1> && algorithms::__is_iterator<InputIterator2>)
#endif // __HAS_CPP20
		_NODISCARD inline bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2) noexcept
		{
			return namespace_function::equal(first1, last1, first2, _STD not_equal_to<> {});
		}

		/* function equal() for 容器、仿函数 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T, typename Function>
			requires(algorithms::__is_container<Container> && algorithms::__is_container<T>)
		_NODISCARD inline bool equal(const Container& con1, const T& con2, Function fun) noexcept
		{
			return namespace_function::equal(__begin_for_container(con1),
											 __end_for_container(con1),
											 __begin_for_container(con2),
											 fun);
		}
#endif	// __HAS_CPP20

		/* function equal() for 容器 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T>
			requires(algorithms::__is_container<Container> && algorithms::__is_container<T>)
		_NODISCARD inline bool equal(const Container& con1, const T& con2) noexcept
		{
			return namespace_function::equal(__begin_for_container(con1),
											 __end_for_container(con1),
											 __begin_for_container(con2),
											 _STD not_equal_to<> {});
		}
#endif // __HAS_CPP20


		/* function fill() 标准版 */
		template <typename ForwardIterator, typename T>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif // __HAS_CPP20
		inline void fill(ForwardIterator first, ForwardIterator last, const T& value) noexcept
		{
			for (; first != last; ++first)
			{
				*first = value;
			}
		}

		/* function fill() for 容器 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T>
			requires(algorithms::__is_container<Container>)
		inline void fill(Container& con, const T& value) noexcept
		{
			namespace_function::fill(__begin_for_container(con), __end_for_container(con), value);
		}
#endif // __HAS_CPP20

		/* function fill_n() 标准版 */
		template <typename OutputIterator, typename Size, typename T>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<OutputIterator>)
#endif // __HAS_CPP20
		inline OutputIterator fill_n(OutputIterator first, Size n, const T& value)
		{
			for (; n > 0; --n, ++first)
			{
				*first = value;
			}

			return first;
		}

		/* function max() for 仿函数 强化版 */
		template <typename T, typename Function>
		inline const T& max(const T& a, const T& b, Function fun) noexcept
		{
#ifdef __HAS_CPP20
			fun = algorithms::__check_fun(fun);
#endif // __HAS_CPP20

			return fun(a, b) ? b : a;
		}

		/* function max() 标准版 */
		template <typename T>
		inline const T& max(const T& a, const T& b) noexcept
		{
			return namespace_function::max(a, b, _STD less<> {});
		}

		/* function max_element() for 仿函数 强化版 */
		template <typename ForwardIterator, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif // __HAS_CPP20
		inline ForwardIterator max_element(ForwardIterator first, ForwardIterator last, Function fun) noexcept
		{
#ifdef __HAS_CPP20
			fun = algorithms::__check_fun(fun);
#endif // __HAS_CPP20

			if (first == last)
			{
				return first;
			}

			ForwardIterator result = first;
			while (++first != last)
			{
				if (fun(*result, *first))
				{
					result = first;
				}
			}

			return result;
		}

		/* function max_element() 标准版 */
		template <typename ForwardIterator>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif // __HAS_CPP20
		inline ForwardIterator max_element(ForwardIterator first, ForwardIterator last) noexcept
		{
			return namespace_function::max_element(first, last, _STD less<> {});
		}

		/* function min() for 仿函数 强化版 */
		template <typename T, typename Function>
		inline const T& min(const T& a, const T& b, Function fun) noexcept
		{
#ifdef __HAS_CPP20
			fun = algorithms::__check_fun(fun);
#endif // __HAS_CPP20

			return fun(a, b) ? b : a;
		}

		/* function min() 标准版 */
		template <typename T>
		inline const T& min(const T& a, const T& b)
		{
			return namespace_function::min(a, b, _STD greater<> {});
		}

		/* function merge() for 仿函数 强化版 */
		template <typename InputIterator1, typename InputIterator2, typename OutputIterator, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator1> && algorithms::__is_iterator<InputIterator2> &&
					 algorithms::__is_iterator<OutputIterator>)
#endif // __HAS_CPP20
		inline OutputIterator merge(InputIterator1 first1,
									InputIterator1 last1,
									InputIterator2 first2,
									InputIterator2 last2,
									OutputIterator result,
									Function	   fun) noexcept
		{
#ifdef __HAS_CPP20
			fun = algorithms::__check_fun(fun);
#endif													   // __HAS_CPP20

			while ((first1 != last1) && (first2 != last2)) // 若两个序列都未完成，则继续
			{
				if (fun(*first2, *first1))				   // 若序列 2 的元素比较小
				{
					*result = *first2;					   // 则记录序列 2 的元素
					++first2;							   // 同时序列 2 前进 1 位
				}
				else									   // 反之同理
				{
					*result = *first1;
					++first1;
				}

				++result;
			}

			// 最后剩余元素拷贝到目的端。（以下两个序列一定至少有一个为空）
			return namespace_copy::copy(first2, last2, namespace_copy::copy(first1, last1, result));
		}

		/* function merge() 标准版 */
		template <typename InputIterator1, typename InputIterator2, typename OutputIterator>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<InputIterator1> && algorithms::__is_iterator<InputIterator2> &&
					 algorithms::__is_iterator<OutputIterator>)
#endif // __HAS_CPP20
		inline OutputIterator merge(InputIterator1 first1,
									InputIterator1 last1,
									InputIterator2 first2,
									InputIterator2 last2,
									OutputIterator result) noexcept
		{
			return namespace_function::merge(first1, last2, first2, last2, result, _STD less<> {});
		}
	} // namespace namespace_function

	// 此处实现 lower_bound()、upper_bound()、equal_range()、binary_search()
	namespace namespace_binary_search
	{
		/* upper_bound() -- 辅助函数 */
		template <typename RandomAccessIterator,
				  typename T,
				  typename Distance,
				  typename Function> // 针对 RandomAccessIterator 的版本
		inline RandomAccessIterator __upper_bound(RandomAccessIterator first,
												  RandomAccessIterator last,
												  const T&			   value,
												  Distance*,
												  __ZH_ITER__ random_access_iterator_tag,
												  Function	  fun)
		{
			Distance			 len	= last - first;
			Distance			 half	= len;
			RandomAccessIterator middle = first;

			while (len > 0)
			{
				half   = len >> 1;
				middle = first + half;

				if (fun(value, *middle))
				{
					len = half;
				}
				else
				{
					first  = middle + 1;
					len	  -= (half + 1);
				}
			}

			return first;
		}

		/* upper_bound() -- 辅助函数 */
		template <typename ForwardIterator,
				  typename T,
				  typename Distance,
				  typename Function> // 针对 ForwardIterator 的版本
		inline ForwardIterator __upper_bound(ForwardIterator first,
											 ForwardIterator last,
											 const T&		 value,
											 Distance*,
											 __ZH_ITER__ forward_iterator_tag,
											 Function	 fun)
		{
			Distance len  = 0;
			Distance half = 0;

			__ZH_ITER__ distance(first, last, len);

			ForwardIterator middle = first;

			while (len > 0)
			{
				half = len >> 1;

				middle = first;
				__ZH_ITER__ advance(middle, half);

				if (fun(value, *middle))
				{
					len = half;
				}
				else
				{
					first = middle;
					++first;

					len -= (half + 1);
				}
			}

			return first;
		}

		/* upper_bound() for 仿函数 强化版 */
		template <typename ForwardIterator, typename T, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif
		inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const T& value, Function fun)
		{
			return namespace_binary_search::__upper_bound(first,
														  last,
														  value,
														  __ZH_ITER__ distance_type(first),
														  __ZH_ITER__ iterator_category(first),
														  algorithms::__check_fun(fun));
		}

		/* upper_bound() 标准版 */
		template <typename ForwardIterator, typename T>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif
		inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const T& value)
		{
			return namespace_binary_search::upper_bound(first,
														last,
														value,
														__ZH_ITER__ distance_type(first),
														__ZH_ITER__ iterator_category(first),
														_STD		less<> {});
		}

		/*-----------------------------------------------------------------------------------------------*/



		/* function lower_bound() -- 辅助函数 */

		// 使用此函数需要保证 仿函数 有效
		template <typename RandomAccessIterator,
				  typename T,
				  typename Distance,
				  typename Function> // 针对 RandomAccessIterator 的版本
		inline RandomAccessIterator __lower_bound(RandomAccessIterator first,
												  RandomAccessIterator last,
												  const T&			   value,
												  Distance*,
												  __ZH_ITER__ random_access_iterator_tag,
												  Function	  fun)
		{
			Distance len  = last - first;
			Distance half = len;

			RandomAccessIterator middle = first;

			while (len > 0)
			{
				half   = len >> 1;
				middle = first + half;

				if (fun(*middle, value))
				{
					first  = middle + 1;
					len	  -= (half + 1);
				}
				else
				{
					len = half;
				}
			}

			return first;
		}

		/* function lower_bound() -- 辅助函数 */

		// 使用此函数需要保证 仿函数 有效
		template <typename ForwardIterator,
				  typename T,
				  typename Distance,
				  typename Function> // 针对 ForwardIterator 的版本
		inline ForwardIterator __lower_bound(ForwardIterator first,
											 ForwardIterator last,
											 const T&		 value,
											 Distance*,
											 __ZH_ITER__ forward_iterator_tag,
											 Function	 fun)
		{
			Distance	len	 = 0;
			Distance	half = 0;
			__ZH_ITER__ distance(first, last, len);

			ForwardIterator middle = first;

			while (len > 0)
			{
				half = len >> 1;

				middle = first;
				__ZH_ITER__ advance(middle, half);

				if (fun(*middle, value))
				{
					first = middle;
					++first;

					len -= (half + 1);
				}
				else
				{
					len = half;
				}
			}

			return first;
		}

		/* function lower_bound() for 仿函数 强化版 */
		template <typename ForwardIterator, typename T, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif
		inline ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const T& value, Function fun)
		{
			// 使用如下函数需要保证 仿函数 有效
			return namespace_binary_search::__lower_bound(first,
														  last,
														  value,
														  __ZH_ITER__ distance_type(first),
														  __ZH_ITER__ iterator_category(first),
														  algorithms::__check_fun(fun));
		}

		/* function lower_bound() */
		template <typename ForwardIterator, typename T>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif
		inline ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const T& value)
		{
			return namespace_binary_search::lower_bound(first,
														last,
														value,
														__ZH_ITER__ distance_type(first),
														__ZH_ITER__ iterator_category(first),
														_STD		less<> {});
		}

		/*-----------------------------------------------------------------------------------------------*/



		/* function equal_range() -- 辅助函数 */

		// 使用此函数需要保证 仿函数 有效
		template <typename RandomAccessIterator,
				  typename T,
				  typename Distance,
				  typename Function> // 针对 RandomAccessIterator 的版本
		inline __ZH_PAIR__ pair<RandomAccessIterator, RandomAccessIterator>
						   __equal_range(RandomAccessIterator first,
										 RandomAccessIterator last,
										 const T&			  value,
										 Distance*,
										 __ZH_ITER__ random_access_iterator_tag,
										 Function	 fun)
		{
			Distance len  = last - first;
			Distance half = len;

			RandomAccessIterator middle = first;
			RandomAccessIterator left	= first;
			RandomAccessIterator right	= first;

			while (len > 0)				 // 如果整个区间尚未迭代完毕
			{
				half   = len >> 1;		 // 找出中间位置
				middle = first + half;	 // 设定中央迭代器

				if (fun(*middle, value)) // 如果 中央元素 < 指定值
				{
					first  = middle + 1; // 将区间缩小（移至后半段），以提高效率
					len	  -= (half + 1);
				}
				else if (fun(value, *middle)) // 如果 中央元素 > 指定值
				{
					len = half;				  // 将区间缩小（移至前半段），以提高效率
				}
				else						  // 如果 中央元素 == 指定值
				{
					left = namespace_binary_search::lower_bound(first, middle, value, fun); // 在前半段寻找
					right = namespace_binary_search::upper_bound(++middle, first + len, value, fun); // 在后半段寻找

					return __ZH_PAIR__ pair<RandomAccessIterator, RandomAccessIterator>(left, right);
				}
			}

			// 整个区间内没有匹配值，则返回一对迭代器--指向第一个 大于指定值 的元素
			return __ZH_PAIR__ pair<RandomAccessIterator, RandomAccessIterator>(first, first);
		}

		/* function equal_range() -- 辅助函数 */

		// 使用此函数需要保证 仿函数 有效
		template <typename ForwardIterator,
				  typename T,
				  typename Distance,
				  typename Function> // 针对 ForwardIterator 的版本
		inline __ZH_PAIR__ pair<ForwardIterator, ForwardIterator> __equal_range(ForwardIterator first,
																				ForwardIterator last,
																				const T&		value,
																				Distance*,
																				__ZH_ITER__ forward_iterator_tag,
																				Function	fun)
		{
			Distance len  = 0;
			Distance half = 0;

			__ZH_ITER__ distance(first, last, len);

			ForwardIterator left   = first;
			ForwardIterator middle = first;
			ForwardIterator right  = first;

			while (len > 0)
			{
				half = len >> 1;

				middle = first;
				__ZH_ITER__ advance(middle, half);

				if (fun(*middle, value))
				{
					first == middle;
					++first;
					len -= (half + 1);
				}
				else if (fun(value, *middle))
				{
					len = half;
				}
				else
				{
					left = namespace_binary_search::lower_bound(first, middle, value, fun);

					__ZH_ITER__ advance(first, len);

					right = namespace_binary_search::upper_bound(++middle, first, value, fun);

					return __ZH_PAIR__ pair<ForwardIterator, ForwardIterator>(left, right);
				}
			}

			return __ZH_PAIR__ pair<ForwardIterator, ForwardIterator>(first, first);
		}

		/* function equal_range() for 仿函数 强化版 */
		template <typename ForwardIterator, typename T, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif
		inline __ZH_PAIR__ pair<ForwardIterator, ForwardIterator>
						   equal_range(ForwardIterator first, ForwardIterator last, const T& value, Function fun)
		{
			// 使用如下函数需要保证 仿函数 有效
			return namespace_binary_search::__equal_range(
				first,
				last,
				value,
				__ZH_ITER__ distance_type(first),
				__ZH_ITER__ iterator_category(first),
				algorithms::__check_fun(fun)); // 根据不同迭代器的类型（category）采取不同的策略
		}

		/* function equal_range() 标准版 */
		template <typename ForwardIterator, typename T>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif
		inline __ZH_PAIR__ pair<ForwardIterator, ForwardIterator>
						   equal_range(ForwardIterator first, ForwardIterator last, const T& value)
		{
			return namespace_binary_search::equal_range(
				first,
				last,
				value,
				__ZH_ITER__ distance_type(first),
				__ZH_ITER__ iterator_category(first),
				_STD		less<> {}); // 根据不同迭代器的类型（category）采取不同的策略
		}

		/* function equal_range() for 容器、仿函数 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T, typename Function>
			requires(algorithms::__is_container<Container>)
		inline auto equal_range(const Container& con, const T& value, Function fun)
		{
			return namespace_binary_search::equal_range(__begin_for_container(con),
														__end_for_container(con),
														value,
														fun);
		}
#endif	// __HAS_CPP20

		/* function equal_range() for 容器 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T>
			requires(algorithms::__is_container<Container>)
		inline auto equal_range(const Container& con, const T& value)
		{
			return namespace_binary_search::equal_range(__begin_for_container(con),
														__end_for_container(con),
														value,
														_STD less<> {});
		}
#endif // __HAS_CPP20

		/*-----------------------------------------------------------------------------------------------*/


		/* function binary_search() for 仿函数 强化版 */
		template <typename ForwardIterator, typename T, typename Function>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif // __HAS_CPP20
		inline bool binary_search(ForwardIterator first, ForwardIterator last, const T& value, Function fun)
		{
			ForwardIterator i = namespace_binary_search::lower_bound(first, last, value, fun);

			// 由于 lower_bound() 会检查 仿函数 的有效性，故此时使用不需要再检查
			return (i != last) && !(fun(value, *i));
		}

		/* function binary_search() 标准版 */
		template <typename ForwardIterator, typename T>
#ifdef __HAS_CPP20
			requires(algorithms::__is_iterator<ForwardIterator>)
#endif // __HAS_CPP20
		inline bool binary_search(ForwardIterator first, ForwardIterator last, const T& value)
		{
			return namespace_binary_search::binary_search(first, last, value, _STD less<> {});
		}

		/* function binary_search() for 容器、仿函数 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T, typename Function>
			requires(algorithms::__is_container<Container>)
		inline bool binary_search(const Container& con, const T& value, Function fun)
		{
			return namespace_binary_search::binary_search(__begin_for_container(con),
														  __end_for_container(con),
														  value,
														  fun);
		}
#endif	// __HAS_CPP20

		/* function binary_search() for 容器 强化版 */
#ifdef __HAS_CPP20
		template <typename Container, typename T>
			requires(algorithms::__is_container<Container>)
		inline bool binary_search(const Container& con, const T& value)
		{
			return namespace_binary_search::binary_search(__begin_for_container(con),
														  __end_for_container(con),
														  value,
														  _STD less<> {});
		}
#endif // __HAS_CPP20
	}  // namespace namespace_binary_search

	// 此处实现 sort()、quick_sort()、insertion_sort()、merge_sort()
	namespace namespace_sort
	{
		// 此处实现 模拟标准库排序
		namespace std_sort
		{
			// 如下三个函数实现了完整的 插入排序

			// 使用此函数需要保证 仿函数 有效
			template <typename RandomAccessIterator,
					  typename T,
					  typename Function> // 插入排序--辅助函数 2 （这是排序的第三步）
			inline void __unguarded_linear_insert(RandomAccessIterator last, T value, Function fun) noexcept
			{
				RandomAccessIterator next = last;
				--next;

				while (fun(value, *next))
				{
					*last = *next;
					last  = next;
					--next;
				}

				*last = value;
			}

			// 使用此函数需要保证 仿函数 有效
			template <typename RandomAccessIterator,
					  typename T,
					  typename Function> // 插入排序--辅助函数 1 （这是排序的第二步）
			inline void
				__linear_insert(RandomAccessIterator first, RandomAccessIterator last, T*, Function fun) noexcept
			{
				T value = *last;

				if (fun(value, *first))
				{
					// TODO: 以期实现自己的 copy_backward()
					_STD copy_backward(first, last, last + 1);
					*first = value;
				}
				else
				{
					std_sort::__unguarded_linear_insert(last, value, algorithms::__check_fun(fun));
				}
			}

			template <typename RandomAccessIterator, typename Function> // 插入排序 （这是排序的第一步）
			inline void __insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun) noexcept
			{
				if (first == last)
				{
					return;
				}
				else
				{
					for (RandomAccessIterator i = first + 1; i != last; ++i)
					{
						std_sort::__linear_insert(first,
												  i,
												  __ZH_ITER__ value_type(first),
												  algorithms::__check_fun(fun));
					}
				}
			}

			/*-----------------------------------------------------------------------------------------*/



			// 如下函数调用了 堆排序
			// 使用此函数需要保证 仿函数 有效
			template <
				typename RandomAccessIterator, // 堆排序
				typename T,
				typename Function> // 排序，使 [first, middle) 中的元素不减有序，[middle,last) 中的元素不做有序保障
			inline void __heap_sort_aux(RandomAccessIterator first,
										RandomAccessIterator middle,
										RandomAccessIterator last,
										T*,
										Function fun) noexcept
			{
				__ZH_HEAP__ make_heap(first, middle);

				for (RandomAccessIterator i = middle; i < last; ++i)
				{
					if (fun(*i, *first))
					{
						__ZH_HEAP__ __pop_heap(first, middle, i, __cove_type(*i, T), __ZH_ITER__ distance_type(first));
					}
				}

				__ZH_HEAP__ sort_heap(first, middle);
			}

			/*-----------------------------------------------------------------------------------------*/



			// 如下两个函数，实现了 快排 的两大核心功能

			// 使用此函数需要保证 仿函数 有效
			template <typename T, typename Function> // 快速排序 -- 返回三点中值
			inline const T& __median(const T& a, const T& b, const T& c, Function fun) noexcept
			{
				if (fun(a, b))
				{
					if (fun(b, c))
					{
						return b;
					}
					else if (fun(a, c))
					{
						return c;
					}
					else
					{
						return a;
					}
				}
				else if (fun(a, c))
				{
					return a;
				}
				else if (fun(b, c))
				{
					return c;
				}
				else
				{
					return b;
				}
			}

			template <typename RandomAccessIterator, typename T, typename Function> // 快速排序 -- 分割
			RandomAccessIterator __unguraded_partition(RandomAccessIterator first,
													   RandomAccessIterator last,
													   const T&				pivot,
													   Function				fun) noexcept
			{
				// 分割的结果最终是：以 piovt 为节点，有如下事实
				// a、节点 pivot 处于正确的位置
				// b、节点 pivot 左边的元素均小于等于 pivot
				// c、节点 pivot 右边的节点均大于等于 pivot
				// d、返回节点 pivot 右边的第一个位置

				while (true)
				{
					while (fun(*first, pivot))
					{
						++first;
					}

					--last;
					while (fun(pivot, *last))
					{
						--last;
					}

					if (!(first < last))
					{
						return first;
					}
					else
					{
						namespace_function::iter_swap(first, last);
						++first;
					}
				}
			}

			/*-----------------------------------------------------------------------------------------*/



			// 如下若干函数，服务于 “两步走” 战略的各个函数

			template <typename RandomAccessIterator,
					  typename T,
					  typename Function> //  sort 第二部分 辅助函数：调用 无边界检查的 插入排序
			inline void __ungurded_insertion_sort_aux(RandomAccessIterator first,
													  RandomAccessIterator last,
													  T*,
													  Function fun) noexcept
			{
				for (RandomAccessIterator i = first; i != last; ++i)
				{
					std_sort::__unguarded_linear_insert(i, __cove_type(*i, T), algorithms::__check_fun(fun));
				}
			}

			template <typename RandomAccessIterator, typename Function> // sort 第二部分 辅助函数：调用 插入排序
			inline void
				__ungurded_insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun) noexcept
			{
				std_sort::__ungurded_insertion_sort_aux(first, last, __ZH_ITER__ value_type(first), fun);
			}

			template <typename RandomAccessIterator,
					  typename Function> // sort 第一部分 辅助函数：递归过深时，改用 “堆排序”
			inline void __heap_sort(RandomAccessIterator first,
									RandomAccessIterator middle,
									RandomAccessIterator last,
									Function			 fun) noexcept
			{
				std_sort::__heap_sort_aux(first,
										  middle,
										  last,
										  __ZH_ITER__ value_type(first),
										  algorithms::__check_fun(fun));
			}

			template <typename Size>		  // sort 第一部分 辅助函数：用于控制分割恶化情况
			inline Size __lg(Size n) noexcept // 找出 2^k <= n 的最大值 k
			{
				auto ans = __init_type(0, Size);

				for (; n > 1; n >>= 1)
				{
					++ans;
				}

				return ans;
			}

			/*------------------------------------------------------------------------------------------------*/



			// 如下两个函数，实现了 sort 的 “两步走” 战略

			// sort -- 第一部分：排序，使之 “几乎有序”
			template <typename RandomAccessIterator, typename T, typename Size, typename Function>
			void __introsort_loop(RandomAccessIterator first,
								  RandomAccessIterator last,
								  T*,
								  Size	   depth_limit,
								  Function fun) noexcept
			{
				// “几乎有序” 的判断标准：需要排序的元素个数足够少，否则视为 “非 ‘几乎有序’ ”
				while ((last - first) > __stl_threshold)
				{
					if (depth_limit == 0) // 递归深度足够深
					{
						std_sort::__heap_sort(first,
											  last,
											  last,
											  fun); // 此时调用 __heap_sort()，实际上调用了一个 “堆排序”

						return;
					}

					--depth_limit;

					// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
					auto pivot = __cove_type(
						std_sort::__median(*first, *(last - 1), *(first + (last - first) / 2), _STD less<> {}),
						T);
					RandomAccessIterator cut =
						std_sort::__unguraded_partition(first, last, pivot, algorithms::__check_fun(fun));

					// 对右半段 递归sort
					std_sort::__introsort_loop(cut, last, __ZH_ITER__ value_type(first), depth_limit, fun);

					// 至此，回到 while 循环，准备对左半段 递归sort
					last = cut;
				}
			}

			// sort -- 第二部分：排序，使 “几乎有序” 蜕变到 “完全有序”
			template <typename RandomAccessIterator, typename Function>
			inline void
				__final_insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun) noexcept
			{
				// 待排序元素个数是否足够多？
				if ((last - first) > __stl_threshold)								 // 是
				{
					std_sort::__insertion_sort(first, first + __stl_threshold, fun); // 对前若干个元素 插入排序
					std_sort::__ungurded_insertion_sort(
						first + __stl_threshold,
						last,
						fun); // 对剩余元素(剩余元素数量一定少于前面的元素数量) 插入排序(无边界检查)
				}
				else											  // 否
				{
					std_sort::__insertion_sort(first, last, fun); // 对这些元素 插入排序
				}
			}

			/*------------------------------------------------------------------------------------------------*/



			// sort() for 仿函数 强化版
			template <typename RandomAccessIterator, typename Function>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void sort(RandomAccessIterator first, RandomAccessIterator last, Function fun) noexcept
			{
				if (first < last) // 真实的排序由以下两个函数完成
				{
					// 排序，使之 “几乎有序”
					std_sort::__introsort_loop(first,
											   last,
											   __ZH_ITER__ value_type(first),
											   std_sort::__lg(last - first) * 2,
											   fun);

					// 排序，使 “几乎有序” 蜕变到 “完全有序”
					std_sort::__final_insertion_sort(first, last, fun);
				}
			}

			// sort() 标准版
			template <typename RandomAccessIterator>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void sort(RandomAccessIterator first, RandomAccessIterator last) noexcept
			{
				std_sort::sort(first, last, _STD less<> {});
			}

			// sort() for 容器、仿函数 强化版
#ifdef __HAS_CPP20
			template <typename Container, typename Function>
				requires(__is_container<Container>)
			inline void sort(Container& con, Function fun) noexcept
			{
				std_sort::sort(__begin_for_container(con), __end_for_container(con), fun);
			}
#endif		// __HAS_CPP20

			// sort() for 容器 强化版
#ifdef __HAS_CPP20
			template <typename Container>
				requires(__is_container<Container>)
			inline void sort(Container& con) noexcept
			{
				std_sort::sort(__begin_for_container(con), __end_for_container(con), _STD less<> {});
			}
#endif	  // __HAS_CPP20
		} // namespace std_sort

		/*-----------------------------------------------------------------------------------------------*/



		// 此处封装 sort 内部完整的 插入排序
		namespace insertion_sort
		{
			// insertion_sort() for 仿函数 强化版
			template <typename RandomAccessIterator, typename Function>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun) noexcept
			{
				namespace_sort::std_sort::__insertion_sort(first, last, fun);
			}

			// insertion_sort() 标准版
			template <typename RandomAccessIterator>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void insertion_sort(RandomAccessIterator first, RandomAccessIterator last) noexcept
			{
				namespace_sort::insertion_sort(first, last, _STD less<> {});
			}

			// insertion_sort()	for 容器、仿函数 强化版
#ifdef __HAS_CPP20
			template <typename Container, typename Function>
				requires(__is_container<Container>)
			inline void insertion_sort(Container& con, Function fun) noexcept
			{
				insertion_sort::insertion_sort(__begin_for_container(con), __end_for_container(con), fun);
			}
#endif		// __HAS_CPP20

			// insertion_sort() for 容器 强化版
#ifdef __HAS_CPP20
			template <typename Container>
				requires(__is_container<Container>)
			inline void insertion_sort(Container& con) noexcept
			{
				insertion_sort::insertion_sort(__begin_for_container(con), __end_for_container(con), _STD less<> {});
			}
#endif	  // __HAS_CPP20
		} // namespace insertion_sort

		/*-----------------------------------------------------------------------------------------------*/



		// 此处实现 归并排序
		namespace merge_sort
		{
			/*
			* 归并排序 未被 sort 采纳，主要是考虑到：
			* 1、需要借助额外的内存
			* 2、在内存之间移动（复制）数据耗时不少
			*/

			// merge_sort() for 仿函数 强化版
			template <typename BidirectionalIterator, typename Function>
#ifdef __HAS_CPP20
				requires(__is_iterator<BidirectionalIterator>)
#endif // __HAS_CPP20
			inline void merge_sort(BidirectionalIterator first, BidirectionalIterator last, Function fun) noexcept
			{
				using difference_type = typename __ZH_ITER__ iterator_traits<BidirectionalIterator>::difference_type;
				difference_type n	  = __ZH_ITER__ distance(first, last);

				if ((n == 0) || (n == 1))
				{
					return;
				}

				BidirectionalIterator mid = first + (n / 2);

				merge_sort::merge_sort(first, mid, fun);
				merge_sort::merge_sort(mid, last, fun);

				// TODO: 以期实现自己的 inplace_merge() ，同时，此前提到插入排序的缺点之一 “借助额外内存” ，就体现在此函数中
				_STD inplace_merge<BidirectionalIterator, Function>(first, mid, last, algorithms::__check_fun(fun));
			}

			// merge_sort() 标准版
			template <typename BidirectionalIterator>
#ifdef __HAS_CPP20
				requires(__is_iterator<BidirectionalIterator>)
#endif // __HAS_CPP20
			inline void merge_sort(BidirectionalIterator first, BidirectionalIterator last) noexcept
			{
				merge_sort::merge_sort(first, last, _STD less<> {});
			}

			// merge_sort() for 容器、仿函数 强化版
#ifdef __HAS_CPP20
			template <typename Container, typename Function>
				requires(__is_container<Container>)
			inline void merge_sort(Container& con, Function fun) noexcept
			{
				merge_sort::merge_sort(__begin_for_container(con), __end_for_container(con), fun);
			}
#endif		// __HAS_CPP20

			// merge_sort() for 容器 强化版
#ifdef __HAS_CPP20
			template <typename Container>
				requires(__is_container<Container>)
			inline void merge_sort(Container& con) noexcept
			{
				merge_sort::merge_sort(__begin_for_container(con), __end_for_container(con), _STD less<> {});
			}
#endif	  // __HAS_CPP20
		} // namespace merge_sort

		/*-----------------------------------------------------------------------------------------------*/



		// 此处实现 快速排序
		namespace quick_sort
		{
			// quick_sort() 辅助函数
			template <typename RandomAccessIterator, typename T, typename Function>
			inline void __quick_sort(RandomAccessIterator first, RandomAccessIterator last, T*, Function fun) noexcept
			{
				while ((last - first) > 1)
				{
					auto pivot = __cove_type(namespace_sort::std_sort::__median(*first,
																				*(last - 1),
																				*(first + (last - first) / 2),
																				_STD less<> {}),
											 T);

					RandomAccessIterator cut =
						namespace_sort::std_sort::__unguraded_partition(first,
																		last,
																		pivot,
																		algorithms::__check_fun(fun));

					if ((last - cut) < (cut - first))
					{
						quick_sort::__quick_sort(cut, last, __ZH_ITER__ value_type(last), fun);
						last = cut;
					}
					else
					{
						quick_sort::__quick_sort(first, cut, __ZH_ITER__ value_type(first), fun);
						first = cut;
					}
				}
			}

			// quick_sort() for 仿函数 强化版
			template <typename RandomAccessIterator, typename Function>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void quick_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun) noexcept
			{
				if (!((last - first) > 1))
				{
					return;
				}

				quick_sort::__quick_sort(first, last, __ZH_ITER__ value_type(first), fun);
			}

			// quick_sort() 标准版
			template <typename RandomAccessIterator>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void quick_sort(RandomAccessIterator first, RandomAccessIterator last) noexcept
			{
				quick_sort::quick_sort(first, last, _STD less<> {});
			}

			// quick_sort() for 容器、仿函数 强化版
#ifdef __HAS_CPP20
			template <typename Container, typename Function>
				requires(__is_container<Container>)
			inline void quick_sort(Container& con, Function fun) noexcept
			{
				quick_sort::quick_sort(__begin_for_container(con), __end_for_container(con), fun);
			}
#endif		// __HAS_CPP20

			// quick_sort() for 容器 强化版
#ifdef __HAS_CPP20
			template <typename Container>
				requires(__is_container<Container>)
			inline void quick_sort(Container& con) noexcept
			{
				quick_sort::quick_sort(__begin_for_container(con), __end_for_container(con), _STD less<> {});
			}
#endif	  // __HAS_CPP20
		} // namespace quick_sort

		// 此处封装 堆排序
		namespace heap_sort
		{
			// heap_sort() for 仿函数 强化版
			template <typename RandomAccessIterator, typename Function>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void heap_sort(RandomAccessIterator first,
								  RandomAccessIterator middle,
								  RandomAccessIterator last,
								  Function			   fun) noexcept
			{
				namespace_sort::std_sort::__heap_sort(first, middle, last, fun);
			}

			// heap_sort() 标准版
			template <typename RandomAccessIterator>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void
				heap_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last) noexcept
			{
				namespace_sort::std_sort::__heap_sort(first, middle, last, _STD less<> {});
			}

			// heap_sort() for 仿函数、无第二区间 强化版
			template <typename RandomAccessIterator, typename Function>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void heap_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun) noexcept
			{
				heap_sort::heap_sort(first, last, last, fun);
			}

			// heap_sort() for 无第二区间 强化版
			template <typename RandomAccessIterator>
#ifdef __HAS_CPP20
				requires(algorithms::__is_iterator<RandomAccessIterator>)
#endif // __HAS_CPP20
			inline void heap_sort(RandomAccessIterator first, RandomAccessIterator last) noexcept
			{
				heap_sort::heap_sort(first, last, last, _STD less<> {});
			}

			// heap_sort() for 容器、仿函数、无第二区间 强化版
#ifdef __HAS_CPP20
			template <typename Container, typename Function>
				requires(algorithms::__is_container<Container>)
			inline void heap_sort(Container& con, Function fun) noexcept
			{
				heap_sort::heap_sort(__begin_for_container(con),
									 __end_for_container(con),
									 __end_for_container(con),
									 fun);
			}
#endif		// __HAS_CPP20

			// heap_sort() for 容器、无第二区间 强化版
#ifdef __HAS_CPP20
			template <typename Container>
				requires(algorithms::__is_container<Container>)
			inline void heap_sort(Container& con) noexcept
			{
				heap_sort::heap_sort(__begin_for_container(con),
									 __end_for_container(con),
									 __end_for_container(con),
									 _STD less<> {});
			}
#endif	  // __HAS_CPP20
		} // namespace heap_sort
	}	  // namespace namespace_sort

	/*-----------------------------------------------------------------------------------------*/



	/* 统一的对外接口 */

	using namespace_binary_search::binary_search;		  // 标准库 binary_search()
	using namespace_binary_search::equal_range;			  // 标准库 equal_range()
	using namespace_binary_search::lower_bound;			  // 标准库 lower_bound()
	using namespace_binary_search::upper_bound;			  // 标准库 upper_bound()

	using namespace_copy::copy;							  // 标准库 copy()

	using namespace_function::equal;					  // 标准库 equal()
	using namespace_function::fill;						  // 标准库 fill()
	using namespace_function::fill_n;					  // 标准库 fill_n()
	using namespace_function::find;						  // 标准库 find()
	using namespace_function::find_if;					  // 标准库 find_if()
	using namespace_function::for_each;					  // 标准库 for_each()
	using namespace_function::iter_swap;				  // 标准库 iter_swap()
	using namespace_function::max;						  // 标准库 mac()
	using namespace_function::max_element;				  // 标准库 max_element()
	using namespace_function::merge;					  // 标准库 merge()
	using namespace_function::min;						  // 标准库 min()
	using namespace_function::swap;						  // 标准库 swap()

	using namespace_sort::heap_sort::heap_sort;			  // 标准库形式的 堆排序
	using namespace_sort::insertion_sort::insertion_sort; // 标准库形式的 插入排序
	using namespace_sort::merge_sort::merge_sort;		  // 标准库形式的 归并排序
	using namespace_sort::quick_sort::quick_sort;		  // 标准库形式的 快速排序
	using namespace_sort::std_sort::sort;				  // 标准库 sort()


#ifdef __ZH_NAMESPACE__
	#undef __ZH_NAMESPACE__
	#undef __ZH_ITER__
	#undef __ZH_PAIR__
	#undef __ZH_HEAP__
#endif // __ZH_NAMESPACE__
} // namespace zhang::algorithms
