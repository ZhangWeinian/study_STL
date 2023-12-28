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
*	目前使用的容器是 MSVC STL 中定义在命名空间 ::std:: 内的，尚未自己实现。这导致 __zh_iter 中萃取函数 iterator_category() 的返回值都是命名空间 ::std:: 中定义的迭代器类型（因为萃取的就是 ::std:: 中的 iterator , 那么返回值当然也是 ::std:: 中定义的类型）。如果使用自定义的迭代器类型，则会有以下报错:
*	“没有适当的转换函数，使类型 ‘ std::xxx_iterator_tag ’ 转换到类型 ‘ zhang::iterator::namespace_iterator::xxx_iterator_tag ’ ”
*
* 如果实现了自定义的容器，那么该怎么写就怎么写
*/



// 此处实现若干纯算法函数
namespace zhang::algorithms
{
#ifdef __HAS_CPP20

	// 预定义一些用于 简写 和 标志识别 的宏
	#ifndef __zh_namespace

		#define __zh_namespace ::zhang::
		#define __zh_pair	   ::zhang::without_book::namespace_pair::
		#define __zh_heap	   ::zhang::sequence_containers::namespace_heap::

	#endif // !__zh_namespace

	/*-----------------------------------------------------------------------------------------*/



	// 此处实现 copy()
	namespace namespace_copy
	{
		/* function __copy_with_random_access_iter() -- 辅助函数 */
		template <__is_random_access_iterator RandomAccessIterator, __is_output_iterator OutputIterator>
		inline OutputIterator __copy_with_random_access_iter(RandomAccessIterator first,
															 RandomAccessIterator last,
															 OutputIterator		  result) noexcept
		{
			using difference_type = __difference_type_for_iter<RandomAccessIterator>;

			for (difference_type i = last - first; i > 0; --i, ++result, ++first) // 以 i 决定循环的次数 -- 速度快
			{
				*result = *first;
			}

			return result;
		}

		/*-----------------------------------------------------------------------------------------*/



		// 此仿函数直接服务于 copy()，它分为一个 完全泛化版本 和两个 偏特化版本

		/* function copy() -- 辅助函数 */
		template <__is_input_iterator InputIterator, __is_output_iterator OutputIterator> // 完全泛化版本
		struct __copy_dispatch
		{
			OutputIterator operator()(InputIterator first, InputIterator last, OutputIterator result) noexcept
			{
				using iter_type_tag = __get_iter_type_tag<InputIterator>;
				constexpr bool is_random_access_iter {
					_STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()>
				};

				// 此处兵分两路：根据迭代器种类的不同，调用不同的 __copy_with_iter_tag() ，为的是不同种类的迭代器所使用的循环条件不同，有快慢之分
				if constexpr (is_random_access_iter)
				{
					// 此处单独划分出一个函数，为的是其他地方也能用到
					return namespace_copy::__copy_with_random_access_iter(__move(first), __move(last), __move(result));
				}
				else
				{
					for (; first != last; ++result, ++first) // 以迭代器相同与否，决定循环是否继续 -- 速度慢
					{
						*result = *first;					 // assignment operator
					}

					return result;
				}
			}
		};

		/* function copy() -- 辅助函数 */
		template <
			typename T> // 偏特化版本1：第一个参数是 const T* 指针形式，第二个参数是 T* 指针形式。另一个偏特化版本两个参数都是 T* 类型的，愚以为没必要…… 具体实现见书 319 页。
		struct __copy_dispatch<const T*, T*>
		{
			T* operator()(const T* first, const T* last, T* result) noexcept
			{
				using pointer_type_tag =
					typename iterator::namespace_iterator::__type_traits<T>::has_trivial_assignment_operator;
				constexpr bool is_trivial_assignment_operator { _STD is_same_v<_STD true_type(), pointer_type_tag()> };

				if (is_trivial_assignment_operator) // 以下版本适用于 “指针所指之对象，具备 trivial assignment operator ”
				{
					__invoke(memmove, result, first, sizeof(T) * (last - first));

					return result + (last - first);
				}
				else // 以下版本适用于 “指针所指之对象，具备 non-trivial assignment operator ”
				{
					return namespace_copy::__copy_with_random_access_iter(__move(first), __move(last), __move(result));
				}
			}
		};

		/*-----------------------------------------------------------------------------------------*/



		// 以下三个函数是 copy() 的对外接口

		/* function copy() 重载1 */
		inline char*
			copy(const char* first,
				 const char* last,
				 char* result) noexcept // 针对原生指针(可视为一种特殊的迭代器) const char* ，机型内存直接拷贝操作
		{
			__invoke(memmove, result, first, last - first);

			return result + (last - first);
		}

		/* function copy() 重载2 */
		inline wchar_t* copy(
			const wchar_t* first,
			const wchar_t* last,
			wchar_t* result) noexcept // 针对原生指针（可视为一种特殊的迭代器）const wchar_t* ，执行内存直接拷贝操作
		{
			__invoke(memmove, result, first, sizeof(wchar_t) * (last - first));

			return result + (last - first);
		}

		/* function copy() 一般泛型 */
		template <__is_input_iterator InputIterator, __is_output_iterator OutputIterator> // 完全泛化版本
		inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result) noexcept
		{
			return namespace_copy::__copy_dispatch<InputIterator, OutputIterator>()(__move(first),
																					__move(last),
																					__move(result));
		}

		/* function copy() for all_容器 强化版 */
		template <__is_range Range>
		inline auto copy(const Range& con1, Range& con2) noexcept
		{
			return namespace_copy::copy(__begin_for_container_move(con1),
										__end_for_container_move(con1),
										__begin_for_container_move(con2));
		}

		/* function copy() for from_容器 强化版 */
		template <__is_range Range, __is_output_iterator OutputIterator>
		inline auto copy(const Range& con, OutputIterator result) noexcept
		{
			return namespace_copy::copy(__begin_for_container_move(con), __end_for_container_move(con), __move(result));
		}
	} // namespace namespace_copy

	/*-----------------------------------------------------------------------------------------*/



	// 此处实现 若干简单函数
	namespace namespace_function
	{
		/* function accumulate() for 仿函数 标准版 */
		template <__is_input_iterator InputIterator, typename T, typename Function>
		inline T accumulate(InputIterator first, InputIterator last, T init, Function fun = _STD plus {}) noexcept
		{
			fun = __check_fun(fun);

			for (; first != last; ++first)
			{
				init = __invoke(fun, init, *first);
			}

			return init;
		}

		/* function accumulate() for 容器、仿函数 强化版 */
		template <__is_range Range, typename T, typename Function>
		inline T accumulate(const Range& con, T init, Function fun = _STD plus {}) noexcept
		{
			return namespace_function::accumulate(__begin_for_container_move(con),
												  __end_for_container_move(con),
												  __move(init),
												  fun);
		}

		/* function count() for 仿函数 标准版 */
		template <__is_input_iterator InputIterator, typename T, class Function = _RANGES equal_to>
		inline auto count(InputIterator first, InputIterator last, const T& value, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			using difference_type = __difference_type_for_iter<InputIterator>;

			difference_type n = 0;
			for (; first != last; ++first)
			{
				if (__invoke(fun, *first, value))
				{
					++n;
				}
			}

			return n;
		}

		/* function count() for 容器、仿函数 强化版 */
		template <__is_range Range, typename T, class Function = _RANGES equal_to>
		inline auto count(Range& con, const T& value, Function fun = {}) noexcept
		{
			return namespace_function::count(__begin_for_container_move(con),
											 __end_for_container_move(con),
											 __move(value),
											 fun);
		}

		/* function itoa() 标准版 */
		template <__is_forward_iterator ForwardIterator, typename T>
		inline void itoa(ForwardIterator first, ForwardIterator last, T value) noexcept
		{
			while (first != last)
			{
				*first++ = value++;
			}
		}

		/* function itoa() for 容器 加强版 */
		template <__is_range Range, typename T>
		inline void itoa(Range& con, T value) noexcept
		{
			namespace_function::itoa(__begin_for_container_move(con), __end_for_container_move(con), __move(value));
		}

		/* function find() for 仿函数 标准版 */
		template <__is_input_iterator InputIterator, typename T, class Function = _RANGES equal_to>
		inline InputIterator find(InputIterator first, InputIterator last, const T& value, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			while ((first != last) && __invoke(fun, *first, value))
			{
				++first;
			}

			return first;
		}

		/* function find() for 仿函数、容器 强化版 */
		template <__is_range Range, typename T, class Function = _RANGES equal_to>
		inline auto find(const Range& con, const T& value, Function fun = {}) noexcept
		{
			return namespace_function::find(__begin_for_container_move(con),
											__end_for_container_move(con),
											__move(value),
											fun);
		}

		/* function find_first_of() for 仿函数 标准版 */
		template <__is_input_iterator	   InputIterator,
				  __is_forward_iterator	   ForwardIterator,
				  class Function = _RANGES equal_to>
		inline InputIterator find_first_of(InputIterator   first1,
										   InputIterator   last1,
										   ForwardIterator first2,
										   ForwardIterator last2,
										   Function		   fun = {}) noexcept
		{
			fun = __check_fun(fun);

			for (; first1 != last1; ++first1)
			{
				for (ForwardIterator i = first2; i != last2; ++i)
				{
					if (__invoke(fun, *first1, *i)) // 如果序列 1 的元素与序列 2 中元素相等
					{
						return first1;
					}
				}
			}

			return last1;
		}

		/* function find_first_of() for 容器、仿函数 强化版 */
		template <__is_range Range, class Function = _RANGES equal_to>
		inline auto find_first_of(const Range& con1, const Range& con2, Function fun = {}) noexcept
		{
			return namespace_function::find_first_of(__begin_for_container_move(con1),
													 __end_for_container_move(con1),
													 __begin_for_container_move(con2),
													 __end_for_container_move(con2),
													 fun);
		}

		/* function swap() 标准版 */
		template <typename T>
		inline void swap(T& a, T& b) noexcept
		{
			T tmp = a;
			a	  = b;
			b	  = tmp;
		}

		/* function iter_swap() 标准版 */
		template <__is_forward_iterator ForwardIterator1, __is_forward_iterator ForwardIterator2>
		inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b) noexcept
		{
			using value_type = __value_type_for_iter<ForwardIterator1>;

			value_type tmp = *a;
			*a			   = *b;
			*b			   = tmp;
		}

		/* function swap_ranges() 标准版 */
		template <__is_forward_iterator ForwardIterator1, __is_forward_iterator ForwardIterator2>
		inline ForwardIterator2
			swap_ranges(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2) noexcept
		{
			for (; first1 != last1; ++first1, ++first2)
			{
				namespace_function::iter_swap(first1, first2);
			}

			return first2;
		}

		/* function swap_ranges() for all_容器 强化版 */
		template <__is_range Range>
		inline auto swap_ranges(Range& con1, Range& con2) noexcept
		{
			return namespace_function::swap_ranges(__begin_for_container_move(con1),
												   __end_for_container_move(con1),
												   __begin_for_container_move(con2));
		}

		/* function swap_ranges() for from_容器 强化版 */
		template <__is_range Range, __is_forward_iterator Iterator>
		inline auto swap_ranges(Range& con, Iterator result) noexcept
		{
			return namespace_function::swap_ranges(__begin_for_container_move(con),
												   __end_for_container_move(con),
												   __move(result));
		}

		/* function for_each() 标准版 */
		template <__is_input_iterator InputIterator, typename Function>
		inline Function for_each(InputIterator first, InputIterator last, Function fun) noexcept
		{
			fun = __check_fun(fun);

			for (; first != last; ++first)
			{
				__invoke(fun, *first);
			}

			return fun;
		}

		/* function for_each() for 容器 强化版 */
		template <__is_range Range, typename Function>
		inline Function for_each(const Range& con, Function fun) noexcept
		{
			return namespace_function::for_each(__begin_for_container_move(con), __end_for_container_move(con), fun);
		}

		/* function equal() for 仿函数 标准版 */
		template <__is_input_iterator	   InputIterator1,
				  __is_input_iterator	   InputIterator2,
				  class Function = _RANGES equal_to>
		inline bool
			equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			for (; first1 != last1; ++first1, ++first2) // 如果序列 1 的元素数量多于序列 2 的元素数量, 那就糟糕了
			{
				if (!(__invoke(fun, *first1, *first2)))
				{
					return false;
				}
			}

			return true;
		}

		/* function equal() for 容器、仿函数 强化版 */
		template <__is_range Container1, __is_range Container2, class Function = _RANGES equal_to>
		inline bool equal(const Container1& con1, const Container2& con2, Function fun = {}) noexcept
		{
			return namespace_function::equal(__begin_for_container_move(con1),
											 __end_for_container_move(con1),
											 __begin_for_container_move(con2),
											 fun);
		}

		/* function fill() 标准版 */
		template <__is_forward_iterator ForwardIterator, typename T>
		inline void fill(ForwardIterator first, ForwardIterator last, const T& value) noexcept
		{
			for (; first != last; ++first)
			{
				*first = value;
			}
		}

		/* function fill_n() 标准版 */
		template <__is_output_iterator OutputIterator, typename Size, typename T>
		inline OutputIterator fill_n(OutputIterator first, Size n, const T& value) noexcept
		{
			for (; n > 0; --n, ++first)
			{
				*first = value;
			}

			return first;
		}

		/* function max() for 仿函数 标准版 */
		template <typename T, class Function = _RANGES less>
		inline const T& max(const T& a, const T& b, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			return __invoke(fun, a, b) ? b : a;
		}

		/* function max_element() for 仿函数 标准版 */
		template <__is_forward_iterator ForwardIterator, class Function = _RANGES less>
		inline ForwardIterator max_element(ForwardIterator first, ForwardIterator last, Function fun = {}) noexcept
		{
			if (first == last)
			{
				return first;
			}

			fun = __check_fun(fun);

			ForwardIterator result { first };
			while (++first != last)
			{
				if (__invoke(fun, *result, *first))
				{
					result = first;
				}
			}

			return result;
		}

		/* function max_element() for 容器，仿函数 强化版 */
		template <__is_range Range, class Function = _RANGES less>
		inline auto max_element(Range& con, Function fun = {}) noexcept
		{
			return namespace_function::max_element(__begin_for_container_move(con), __end_for_container_move(con), fun);
		}

		/* function min() for 仿函数 标准版 */
		template <typename T, class Function = _RANGES greater>
		inline const T& min(const T& a, const T& b, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			return __invoke(fun, a, b) ? b : a;
		}

		/* function min_element() for 仿函数 标准版 */
		template <__is_forward_iterator ForwardIterator, typename FUnction = _RANGES greater>
		inline ForwardIterator min_element(ForwardIterator first, ForwardIterator last, FUnction fun = {}) noexcept
		{
			if (first == last)
			{
				return first;
			}

			fun = __check_fun(fun);

			ForwardIterator result { first };
			while (++first != last)
			{
				if (__invoke(fun, *result, *first))
				{
					result = first;
				}
			}

			return result;
		}

		/* function min_element() for 仿函数、容器 强化版 */
		template <__is_range Range, class Function = _RANGES greater>
		inline auto min_element(Range& con, Function fun = {}) noexcept
		{
			return namespace_function::min_element(__begin_for_container_move(con), __end_for_container_move(con), fun);
		}

		/* function merge() for 仿函数 标准版 */
		template <__is_input_iterator	   InputIterator1,
				  __is_input_iterator	   InputIterator2,
				  __is_output_iterator	   OutputIterator,
				  class Function = _RANGES less>
		inline OutputIterator merge(InputIterator1 first1,
									InputIterator1 last1,
									InputIterator2 first2,
									InputIterator2 last2,
									OutputIterator result,
									Function	   fun = {}) noexcept
		{
			fun = __check_fun(fun);

			while ((first1 != last1) && (first2 != last2)) // 若两个序列都未完成，则继续
			{
				if (__invoke(fun, *first2, *first1))	   // 若序列 2 的元素比较小
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
			return namespace_copy::copy(__move(first2),
										__move(last2),
										namespace_copy::copy(__move(first1), __move(last1), __move(result)));
		}

		/* function merge() for 容器、仿函数 强化版 */
		template <__is_range			   Container1,
				  __is_range			   Container2,
				  __is_output_iterator	   OutputIterator,
				  class Function = _RANGES less>
		inline auto
			merge(const Container1& con1, const Container2& con2, OutputIterator result, Function fun = {}) noexcept
		{
			return namespace_function::merge(__begin_for_container_move(con1),
											 __end_for_container_move(con1),
											 __begin_for_container_move(con2),
											 __end_for_container_move(con2),
											 __move(result),
											 fun);
		}

		/* function transform() for 仿函数 标准版 */
		template <__is_input_iterator InputIterator, __is_output_iterator OutputIterator, typename Function>
		inline OutputIterator transform(InputIterator  first,
										InputIterator  last,
										OutputIterator result,
										Function fun = _STD identity {}) noexcept
		{
			fun = __check_fun(fun);

			for (; first != last; ++first, ++result)
			{
				*result = __invoke(fun, *first);
			}

			return result;
		}

		/* function transform() for from_容器 强化版 */
		template <__is_range Range, __is_output_iterator Iterator, typename Function>
		inline Iterator transform(Range& con, Iterator result, Function fun = _STD identity {}) noexcept
		{
			return namespace_function::transform(__begin_for_container_move(con),
												 __end_for_container_move(con),
												 __move(result),
												 fun);
		}

		/* function transform() for all_容器 强化版 */
		template <__is_range Range, typename Function>
		inline auto transform(Range& con1, Range& con2, Function fun = _STD identity {}) noexcept
		{
			return namespace_function::transform(__begin_for_container_move(con1),
												 __end_for_container_move(con1),
												 __begin_for_container_move(con2),
												 fun);
		}
	} // namespace namespace_function

	/*-----------------------------------------------------------------------------------------*/



	// 此处实现 sort()、quick_sort()、insertion_sort()、merge_sort()、heap_sort()、nth_element()
	namespace namespace_sort
	{
		// 此处实现 模拟标准库排序
		namespace std_sort
		{
			// 如下三个函数实现了完整的 插入排序

			// 使用此函数需要保证 仿函数 有效
			template <__is_random_access_iterator RandomAccessIterator,
					  typename T,
					  class Function = _RANGES less> // 插入排序--辅助函数 2 （这是排序的第三步）
			inline void __unguarded_insertion_sort(RandomAccessIterator last, T value, Function fun = {}) noexcept
			{
				RandomAccessIterator next { last };
				--next;

				while (__invoke(fun, value, *next))
				{
					*last = *next;
					last  = next;
					--next;
				}

				*last = value;
			}

			// 使用此函数需要保证 仿函数 有效
			template <__is_random_access_iterator RandomAccessIterator,
					  class Function = _RANGES	  less> // 插入排序--辅助函数 1 （这是排序的第二步）
			inline void __guarded_insertion_sort(RandomAccessIterator first,
												 RandomAccessIterator last,
												 Function			  fun = {}) noexcept
			{
				using value_type = __value_type_for_iter<RandomAccessIterator>;

				value_type value { *last };

				if (__invoke(fun, value, *first))
				{
					// TODO: 以期实现自己的 copy_backward()
					_RANGES copy_backward(first, last, last + 1);

					*first = value;
				}
				else
				{
					std_sort::__unguarded_insertion_sort(__move(last), __move(value), __check_fun(fun));
				}
			}

			template <__is_random_access_iterator RandomAccessIterator,
					  class Function = _RANGES	  less> // 插入排序 （这是排序的第一步）
			inline void
				__insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}) noexcept
			{
				if (first == last)
				{
					return;
				}
				else
				{
					for (RandomAccessIterator i = first + 1; i != last; ++i)
					{
						std_sort::__guarded_insertion_sort(first, i, __check_fun(fun));
					}
				}
			}

			/*-----------------------------------------------------------------------------------------*/



			// 如下函数调用了 堆排序
			// 使用此函数需要保证 仿函数 有效
			template <__is_random_access_iterator RandomAccessIterator, // 堆排序
					  class Function =
						  _RANGES less> // 排序，使 [first, middle) 中的元素不减有序，[middle,last) 中的元素不做有序保障
			inline void __unguarded_heap_sort(RandomAccessIterator first,
											  RandomAccessIterator middle,
											  RandomAccessIterator last,
											  Function			   fun = {}) noexcept
			{
				using value_type = __value_type_for_iter<RandomAccessIterator>;

				__zh_heap make_heap(first, middle, fun);

				for (RandomAccessIterator i = middle; i < last; ++i)
				{
					if (__invoke(fun, *i, *first))
					{
						__zh_heap __pop_heap(first, middle, i, __cove_type(*i, value_type), fun);
					}
				}

				__zh_heap sort_heap(__move(first), __move(middle), fun);
			}

			/*-----------------------------------------------------------------------------------------*/



			// 如下两个函数，实现了 快排 的两大核心功能

			// 函数 1
			// 使用此函数需要保证 仿函数 有效
			template <typename T, class Function = _RANGES less> // 快速排序 -- 返回三点中值
			inline const T& __get_median(const T& a, const T& b, const T& c, Function fun = {}) noexcept
			{
				if (__invoke(fun, a, b))
				{
					if (__invoke(fun, b, c))
					{
						return b;
					}
					else if (__invoke(fun, a, c))
					{
						return c;
					}
					else
					{
						return a;
					}
				}
				else if (__invoke(fun, a, c))
				{
					return a;
				}
				else if (__invoke(fun, b, c))
				{
					return c;
				}
				else
				{
					return b;
				}
			}

			// 函数 2
			template <__is_random_access_iterator RandomAccessIterator,
					  typename T,
					  class Function = _RANGES less> // 快速排序 -- 分割
			RandomAccessIterator __unguraded_partition(RandomAccessIterator first,
													   RandomAccessIterator last,
													   const T&				pivot,
													   Function				fun = {}) noexcept
			{
				// 分割的结果最终是：以 piovt 为节点，有如下事实
				// a、节点 pivot 处于正确的位置
				// b、节点 pivot 左边的元素均小于等于 pivot
				// c、节点 pivot 右边的节点均大于等于 pivot
				// d、返回节点 pivot 右边的第一个位置

				while (true)
				{
					while (__invoke(fun, *first, pivot))
					{
						++first;
					}

					--last;
					while (__invoke(fun, pivot, *last))
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

			template <__is_random_access_iterator RandomAccessIterator,
					  class Function = _RANGES less> //  sort 第二部分 辅助函数：调用 无边界检查的 插入排序
			inline void __call_ungurded_insertion_sort(RandomAccessIterator first,
													   RandomAccessIterator last,
													   Function				fun = {}) noexcept
			{
				using value_type = __value_type_for_iter<RandomAccessIterator>;

				for (RandomAccessIterator i = first; i != last; ++i)
				{
					std_sort::__unguarded_insertion_sort(i, __cove_type(*i, value_type), __check_fun(fun));
				}
			}

			template <__is_random_access_iterator RandomAccessIterator,
					  class Function = _RANGES	  less> // sort 第二部分 辅助函数：调用 插入排序
			inline void __obtain_parameter_for_unguarded_insertion_sort(RandomAccessIterator first,
																		RandomAccessIterator last,
																		Function			 fun = {}) noexcept
			{
				std_sort::__call_ungurded_insertion_sort(__move(first), __move(last), fun);
			}

			template <__is_random_access_iterator RandomAccessIterator,
					  class Function = _RANGES less> // sort 第一部分 辅助函数：递归过深时，改用 “堆排序”
			inline void __heap_sort(RandomAccessIterator first,
									RandomAccessIterator middle,
									RandomAccessIterator last,
									Function			 fun = {}) noexcept
			{
				std_sort::__unguarded_heap_sort(__move(first), __move(middle), __move(last), __check_fun(fun));
			}

			template <typename Size> // sort 第一部分 辅助函数：用于控制分割恶化情况
			inline Size __get_depth_limit(Size n) noexcept // 找出 2^k <= n 的最大值 k
			{
				Size ans { 0 };

				for (; n > 1; n >>= 1)
				{
					++ans;
				}

				return ans;
			}

			/*------------------------------------------------------------------------------------------------*/



			// 如下两个函数，实现了 sort 的 “两步走” 战略

			// sort -- 第一部分：排序，使之 “几乎有序”
			template <__is_random_access_iterator RandomAccessIterator, typename Size, class Function = _RANGES less>
			void __introsort_loop_sort(RandomAccessIterator first,
									   RandomAccessIterator last,
									   Size					depth_limit,
									   Function				fun = {}) noexcept
			{
				using value_type = __value_type_for_iter<RandomAccessIterator>;

				// “几乎有序” 的判断标准：需要排序的元素个数足够少，否则视为 “非 ‘几乎有序’ ”
				while (__stl_threshold < (last - first)) // “几乎有序”
				{
					if (depth_limit == 0)				 // 递归深度足够深
					{
						std_sort::__heap_sort(first,
											  last,
											  last,
											  fun); // 此时调用 __heap_sort()，实际上调用了一个 “堆排序”

						return;
					}

					--depth_limit;

					// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
					auto				 pivot { __cove_type(
						std_sort::__get_median(*first, *(last - 1), *(first + (last - first) / 2), fun),
						value_type) };
					RandomAccessIterator cut { std_sort::__unguraded_partition(first, last, pivot, __check_fun(fun)) };

					// 对右半段 递归sort
					std_sort::__introsort_loop_sort(cut, last, depth_limit, fun);

					// 至此，回到 while 循环，准备对左半段递归排序
					last = cut;
				}
			}

			// sort -- 第二部分：排序，使 “几乎有序” 蜕变到 “完全有序”
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void __final_insertion_sort(RandomAccessIterator first,
											   RandomAccessIterator last,
											   Function				fun = {}) noexcept
			{
				// 待排序元素个数是否足够多？
				if (__stl_threshold < (last - first))								 // 是
				{
					std_sort::__insertion_sort(first, first + __stl_threshold, fun); // 对前若干个元素 插入排序

					std_sort::__obtain_parameter_for_unguarded_insertion_sort(
						__move(first + __stl_threshold),
						__move(last),
						fun); // 对剩余元素(剩余元素数量一定少于前面的元素数量) 插入排序(无边界检查)
				}
				else															  // 否
				{
					std_sort::__insertion_sort(__move(first), __move(last), fun); // 对这些元素 插入排序
				}
			}

			/*------------------------------------------------------------------------------------------------*/



			// sort() for 仿函数 标准版
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void sort(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}) noexcept
			{
				if (first < last) // 真实的排序由以下两个函数完成
				{
					// 排序，使之 “几乎有序”
					std_sort::__introsort_loop_sort(first, last, std_sort::__get_depth_limit(last - first) * 2, fun);

					// 排序，使 “几乎有序” 蜕变到 “完全有序”
					std_sort::__final_insertion_sort(__move(first), __move(last), fun);
				}
			}

			// sort() for 容器、仿函数 强化版
			template <__is_range Range, class Function = _RANGES less>
			inline void sort(Range& con, Function fun = {}) noexcept
			{
				std_sort::sort(__begin_for_container_move(con), __end_for_container_move(con), fun);
			}
		} // namespace std_sort

		/*-----------------------------------------------------------------------------------------------*/



		// 此处封装 sort 内部完整的 插入排序
		namespace insertion_sort
		{
			// insertion_sort() for 仿函数 标准版
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void
				insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}) noexcept
			{
				namespace_sort::std_sort::__insertion_sort(__move(first), __move(last), fun);
			}

			// insertion_sort()	for 容器、仿函数 强化版
			template <__is_range Range, class Function = _RANGES less>
			inline void insertion_sort(Range& con, Function fun = {}) noexcept
			{
				insertion_sort::insertion_sort(__begin_for_container_move(con), __end_for_container_move(con), fun);
			}
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
			template <__is_bidirectional_iterator BidirectionalIterator, class Function = _RANGES less>
			inline void merge_sort(BidirectionalIterator first, BidirectionalIterator last, Function fun = {}) noexcept
			{
				using difference_type = __difference_type_for_iter<BidirectionalIterator>;
				difference_type n	  = _RANGES distance(first, last);

				if ((n == 0) || (n == 1))
				{
					return;
				}

				BidirectionalIterator mid = first + (n / 2);

				merge_sort::merge_sort(first, mid, fun);
				merge_sort::merge_sort(mid, last, fun);

				// TODO: 以期实现自己的 inplace_merge() ，同时，此前提到插入排序的缺点之一 “借助额外内存” ，就体现在此函数中
				_RANGES inplace_merge(__move(first), __move(mid), __move(last), __check_fun(fun));
			}

			// merge_sort() for 容器、仿函数 强化版
			template <__is_range Range, class Function = _RANGES less>
			inline void merge_sort(Range& con, Function fun = {}) noexcept
			{
				merge_sort::merge_sort(__begin_for_container_move(con), __end_for_container_move(con), fun);
			}
		} // namespace merge_sort

		/*-----------------------------------------------------------------------------------------------*/



		// 此处实现 快速排序
		namespace quick_sort
		{
			// quick_sort() 辅助函数
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void __quick_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}) noexcept
			{
				using value_type = __value_type_for_iter<RandomAccessIterator>;

				while (1 < (last - first))
				{
					auto pivot { __cove_type(
						namespace_sort::std_sort::__get_median(*first, *(last - 1), *(first + (last - first) / 2), fun),
						value_type) };

					RandomAccessIterator cut {
						namespace_sort::std_sort::__unguraded_partition(first, last, pivot, __check_fun(fun))
					};

					if ((last - cut) < (cut - first))
					{
						quick_sort::__quick_sort(cut, last, fun);
						last = cut;
					}
					else
					{
						quick_sort::__quick_sort(first, cut, fun);
						first = cut;
					}
				}
			}

			// quick_sort() for 仿函数 标准版
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void quick_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}) noexcept
			{
				if (!(1 < (last - first)))
				{
					return;
				}

				quick_sort::__quick_sort(__move(first), __move(last), fun);
			}

			// quick_sort() for 容器、仿函数 强化版
			template <__is_range Range, class Function = _RANGES less>
			inline void quick_sort(Range& con, Function fun = {}) noexcept
			{
				quick_sort::quick_sort(__begin_for_container_move(con), __end_for_container_move(con), fun);
			}
		} // namespace quick_sort

		/*-----------------------------------------------------------------------------------------------*/



		// 此处封装 堆排序
		namespace heap_sort
		{
			// heap_sort() for 仿函数 标准版
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void heap_sort(RandomAccessIterator first,
								  RandomAccessIterator middle,
								  RandomAccessIterator last,
								  Function			   fun = {}) noexcept
			{
				namespace_sort::std_sort::__heap_sort(__move(first), __move(middle), __move(last), fun);
			}

			// heap_sort() for 仿函数、无第二区间 强化版
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void heap_sort(RandomAccessIterator first, RandomAccessIterator last, Function fun = {}) noexcept
			{
				heap_sort::heap_sort(first, last, last, fun);
			}

			// heap_sort() for 容器、仿函数、无第二区间 强化版
			template <__is_range Range, class Function = _RANGES less>
			inline void heap_sort(Range& con, Function fun = {}) noexcept
			{
				heap_sort::heap_sort(__begin_for_container(con),
									 __end_for_container(con),
									 __end_for_container(con),
									 fun);
			}
		} // namespace heap_sort

		/*-----------------------------------------------------------------------------------------------*/


		// 此处封装 与排序有关的其他简单函数
		namespace sort_function
		{
			/* function nth_element() for 仿函数 标准版 */
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void nth_element(RandomAccessIterator first,
									RandomAccessIterator nth,
									RandomAccessIterator last,
									Function			 fun = {}) noexcept
			{
				fun = __check_fun(fun);

				using value_type = __value_type_for_iter<RandomAccessIterator>;

				while (3 < (last - first)) // 如果 长度 > 3
				{
					// 采用 “三点中值”
					// 返回一个迭代器，指向分割后的右段第一个元素

					RandomAccessIterator cut { namespace_sort::std_sort::__unguraded_partition(
						first,
						last,
						__cove_type(namespace_sort::std_sort::__get_median(*first,
																		   *(last - 1),
																		   *(first + (last - first) / 2),
																		   fun),
									value_type),
						fun) };

					if (nth < cut)	 // 如果 指定位置 < 右段起点，（即 nth 位于右段）
					{
						first = cut; // 对右段实施分割
					}
					else			 // 否则（nth 位于左段）
					{
						last = cut;	 // 对左段实施分割
					}
				}

				namespace_sort::std_sort::__insertion_sort(__move(first), __move(last), fun);
			}

			/* function partial_sort() for 仿函数 标准版 */
			template <__is_random_access_iterator RandomAccessIterator, class Function = _RANGES less>
			inline void partial_sort(RandomAccessIterator first,
									 RandomAccessIterator middle,
									 RandomAccessIterator last,
									 Function			  fun = {}) noexcept
			{
				fun = __check_fun(fun);

				using value_type = __value_type_for_iter<RandomAccessIterator>;

				__zh_heap make_heap(first, middle, fun);

				for (RandomAccessIterator i { middle }; i != last; ++i)
				{
					if (__invoke(fun, *i, *first))
					{
						__zh_heap __pop_heap(first, middle, i, __cove_type(*i, value_type), fun);
					}
				}

				__zh_heap sort_heap(__move(first), __move(middle), fun);
			}

			/* function partial_sort() for 容器、仿函数 强化版 */
			template <__is_range Range, class Function = _RANGES less>
			inline void partial_sort(Range& con, Function fun = {}) noexcept
			{
				sort_function::partial_sort(__begin_for_container_move(con), __end_for_container_move(con), fun);
			}
		} // namespace sort_function

		  /*-----------------------------------------------------------------------------------------------*/
	} // namespace namespace_sort

	/*-----------------------------------------------------------------------------------------*/



	// 此处实现 lower_bound()、upper_bound()、equal_range()、binary_search()
	namespace namespace_binary_search
	{
		/* upper_bound() for 仿函数 标准版 */
		template <__is_forward_iterator ForwardIterator, typename T, class Function = _RANGES less>
		inline ForwardIterator
			upper_bound(ForwardIterator first, ForwardIterator last, const T& value, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			using difference_type = __difference_type_for_iter<ForwardIterator>;
			using iter_type_tag	  = __get_iter_type_tag<ForwardIterator>;
			constexpr bool is_random_access_iter { _STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()> };

			difference_type len { 0 };
			difference_type half { 0 };

			ForwardIterator middle { first };

			if constexpr (is_random_access_iter)
			{
				len = last - first;
			}
			else
			{
				len = _RANGES distance(first, last);
			}

			while (0 < len)
			{
				half = len >> 1;

				if constexpr (is_random_access_iter)
				{
					middle = first + half;
				}
				else
				{
					middle = first;
					_RANGES advance(middle, half);
				}

				if (__invoke(fun, value, *middle))
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

		/* upper_bound() for 容器、仿函数 强化版 */
		template <__is_range Range, typename T, class Function = _RANGES less>
		inline auto upper_bound(const Range& con, const T& value, Function fun = {}) noexcept
		{
			return namespace_binary_search::upper_bound(__begin_for_container_move(con),
														__end_for_container_move(con),
														__move(value),
														fun);
		}

		/*-----------------------------------------------------------------------------------------------*/



		/* function lower_bound() for 仿函数 标准版 */
		template <__is_forward_iterator ForwardIterator, typename T, class Function = _RANGES less>
		inline ForwardIterator
			lower_bound(ForwardIterator first, ForwardIterator last, const T& value, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			using difference_type = __difference_type_for_iter<ForwardIterator>;
			using iter_type_tag	  = __get_iter_type_tag<ForwardIterator>;
			constexpr bool is_random_access_iter { _STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()> };

			difference_type len { 0 };
			difference_type half { 0 };

			ForwardIterator middle { first };

			if constexpr (is_random_access_iter)
			{
				len = last - first;
			}
			else
			{
				len = _RANGES distance(first, last);
			}

			while (0 < len)
			{
				half = len >> 1;

				if constexpr (is_random_access_iter)
				{
					middle = first + half;
				}
				else
				{
					middle = first;
					_RANGES advance(middle, half);
				}

				if (__invoke(fun, *middle, value))
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

		/* function lower_bound() for 容器、仿函数 强化版 */
		template <__is_range Range, typename T, class Function = _RANGES less>
		inline auto lower_bound(const Range& con, const T& value, Function fun = {}) noexcept
		{
			return namespace_binary_search::lower_bound(__begin_for_container_move(con),
														__end_for_container_move(con),
														__move(value),
														fun);
		}

		/*-----------------------------------------------------------------------------------------------*/



		/* function equal_range() for 仿函数 标准版 */
		template <__is_forward_iterator ForwardIterator, typename T, class Function = _RANGES less>
		inline __zh_pair pair<ForwardIterator, ForwardIterator>
			equal_range(ForwardIterator first, ForwardIterator last, const T& value, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			using difference_type = __difference_type_for_iter<ForwardIterator>;
			using iter_type_tag	  = __get_iter_type_tag<ForwardIterator>;
			constexpr bool is_random_access_iter { _STD is_same_v<_STD random_access_iterator_tag(), iter_type_tag()> };

			difference_type len { 0 };
			difference_type half { 0 };

			ForwardIterator left { first };
			ForwardIterator middle { first };
			ForwardIterator right { first };

			if constexpr (is_random_access_iter)
			{
				len = last - first;
			}
			else
			{
				len = _RANGES distance(first, last);
			}

			while (0 < len)							 // 如果整个区间尚未迭代完毕
			{
				half = len >> 1;					 // 找出中间位置

				if constexpr (is_random_access_iter) // 设定中央迭代器
				{
					middle = first + half;
				}
				else
				{
					middle = first;
					_RANGES advance(middle, half);
				}

				if (__invoke(fun, *middle, value)) // 如果 中央元素 < 指定值
				{
					first = middle;
					++first; // 将区间缩小（移至后半段），以提高效率

					len -= (half + 1);
				}
				else if (__invoke(fun, value, *middle)) // 如果 中央元素 > 指定值
				{
					len = half;							// 将区间缩小（移至前半段），以提高效率
				}
				else									// 如果 中央元素 == 指定值
				{
					left  = namespace_binary_search::lower_bound(first, middle, value, fun); // 在前半段寻找
					right = namespace_binary_search::upper_bound(__move(++middle),
																 __move(first + len),
																 __move(value),
																 fun); // 在后半段寻找

					return __zh_pair pair { __move(left), __move(right) };
				}
			}

			// 整个区间内没有匹配值，则返回一对迭代器--指向第一个 大于指定值 的元素
			return __zh_pair pair { first, first };
		}

		/* function equal_range() for 容器、仿函数 强化版 */
		template <__is_range Range, typename T, class Function = _RANGES less>
		inline auto equal_range(const Range& con, const T& value, Function fun = {}) noexcept
		{
			return namespace_binary_search::equal_range(__begin_for_container_move(con),
														__end_for_container_move(con),
														__move(value),
														fun);
		}

		/*-----------------------------------------------------------------------------------------------*/


		/* function binary_search() for 仿函数 标准版 */
		template <__is_forward_iterator ForwardIterator, typename T, class Function = _RANGES less>
		inline bool
			binary_search(ForwardIterator first, ForwardIterator last, const T& value, Function fun = {}) noexcept
		{
			fun = __check_fun(fun);

			ForwardIterator i = namespace_binary_search::lower_bound(first, last, value, fun);

			return (i != last) && !(__invoke(fun, value, *i));
		}

		/* function binary_search() for 容器、仿函数 强化版 */
		template <__is_range Range, typename T, class Function = _RANGES less>
		inline bool binary_search(const Range& con, const T& value, Function fun = {}) noexcept
		{
			return namespace_binary_search::binary_search(__begin_for_container_move(con),
														  __end_for_container_move(con),
														  __move(value),
														  fun);
		}
	} // namespace namespace_binary_search

	/*-----------------------------------------------------------------------------------------*/



	/* 统一的对外接口 */

	using namespace_copy::copy;							  // 标准库 copy()

	using namespace_function::accumulate;				  // 标准库 accumulate()
	using namespace_function::count;					  // 标准库 count() ，包含 count_if()
	using namespace_function::equal;					  // 标准库 equal()
	using namespace_function::fill;						  // 标准库 fill()
	using namespace_function::fill_n;					  // 标准库 fill_n()
	using namespace_function::find;						  // 标准库 find() ，包含 find_if()
	using namespace_function::find_first_of;			  // 标准库 find_first_of()
	using namespace_function::for_each;					  // 标准库 for_each()
	using namespace_function::iter_swap;				  // 标准库 iter_swap()
	using namespace_function::itoa;						  // SGI itoa()
	using namespace_function::max;						  // 标准库 mac()
	using namespace_function::max_element;				  // 标准库 max_element()
	using namespace_function::merge;					  // 标准库 merge()
	using namespace_function::min;						  // 标准库 min()
	using namespace_function::min_element;				  // 标准库 min_element()
	using namespace_function::swap;						  // 标准库 swap()
	using namespace_function::swap_ranges;				  // 标准库 swap_ranges()
	using namespace_function::transform;				  // 标准库 tramsform()

	using namespace_sort::heap_sort::heap_sort;			  // 仿照标准库形式的 堆排序
	using namespace_sort::insertion_sort::insertion_sort; // 仿照标准库形式的 插入排序
	using namespace_sort::merge_sort::merge_sort;		  // 仿照标准库形式的 归并排序
	using namespace_sort::quick_sort::quick_sort;		  // 仿照标准库形式的 快速排序
	using namespace_sort::sort_function::nth_element;	  // 标准库 nth_element()
	using namespace_sort::sort_function::partial_sort;	  // 标准库 partial_sort()
	using namespace_sort::std_sort::sort;				  // 标准库 sort()

	using namespace_binary_search::binary_search;		  // 标准库 binary_search()
	using namespace_binary_search::equal_range;			  // 标准库 equal_range()
	using namespace_binary_search::lower_bound;			  // 标准库 lower_bound()
	using namespace_binary_search::upper_bound;			  // 标准库 upper_bound()

	/*-----------------------------------------------------------------------------------------*/



	#ifdef __zh_namespace
		#undef __zh_namespace
		#undef __zh_pair
		#undef __zh_heap
	#endif // __zh_namespace

#endif	   // __HAS_CPP20
} // namespace zhang::algorithms
