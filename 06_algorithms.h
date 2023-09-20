#include "00_basicFile.h"

// 此处实现若干函数
namespace zhang::algorithms
{
	// 此处实现若干简单函数
	namespace namespace_function
	{
		/* function iter_swap */
		template <typename ForwardIterator1, typename ForwardIterator2, typename T>
		inline void __swap(ForwardIterator1 a, ForwardIterator2 b, T*)
		{
			T tmp = *a;
			*a	  = *b;
			*b	  = tmp;
		}

		template <typename ForwardIterator1, typename ForwardIterator2>
		inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b)
		{
			__swap(a, b, ::zhang::iterator::value_type(a));
		}

		/* function for_each */
		template <typename InputIterator, typename Function>
		Function for_each(InputIterator first, InputIterator last, Function f)
		{
			for (; first != last; ++first)
			{
				f(*first);
			}

			return f;
		}

	} // namespace namespace_function

	// 此处仅实现一个 sort
	namespace namespace_sort
	{
		// 如下三个函数实现了完整的 插入排序

		template <typename RandomAccessIterator, typename T> // 插入排序--辅助函数 2 （这是排序的第三步）
		inline void __unguarded_linear_insert(RandomAccessIterator last, T value)
		{
			RandomAccessIterator next = last;
			--next;

			while (*next > value)
			{
				*last = *next;
				last  = next;
				--next;
			}

			*last = value;
		}

		template <typename RandomAccessIterator, typename T> // 插入排序--辅助函数 1 （这是排序的第二步）
		inline void __linear_insert(RandomAccessIterator first, RandomAccessIterator last, T*)
		{
			T value = *last;

			if (*first > value)
			{
				// HACK: 以期实现自己的 copy_backward()
				_STD copy_backward(first, last, last + 1);
				*first = value;
			}
			else
			{
				__unguarded_linear_insert(last, value);
			}
		}

		template <typename RandomAccessIterator> // 插入排序 （这是排序的第一步）
		inline void __insertion_sort(RandomAccessIterator first, RandomAccessIterator last)
		{
			if (first >= last)
			{
				return;
			}
			else
			{
				for (RandomAccessIterator i = first + 1; i != last; ++i)
				{
					__linear_insert(first, i, ::zhang::iterator::value_type(first));
				}
			}
		}

		/*-----------------------------------------------------------------------------------------*/



		// 如下函数调用了 堆排序

		template <typename RandomAccessIterator, // 堆排序
				  typename T> // 排序，使 [first, middle) 中的元素不减有序，[middle,last) 中的元素不做有序保障
		inline void
			__partial_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, T*)
		{
			::zhang::sequence_containers::make_heap(first, middle);

			for (RandomAccessIterator i = middle; i < last; ++i)
			{
				if (*first > *i)
				{
					::zhang::sequence_containers::push_heap(first, middle);

					/*::zhang::sequence_containers::push_heap(first,
															middle,
															i,
															_cove_type(*i, T),
															::zhang::iterator::distance_type(first));*/
				}

				::zhang::sequence_containers::sort_heap(first, middle);
			}
		}

		/*-----------------------------------------------------------------------------------------*/



		// 如下两个函数，实现了 快排 的两大核心功能

		template <typename T> // 快速排序 -- 返回三点中值
		inline const T& __median(const T& a, const T& b, const T& c)
		{
			if (a < b)
			{
				if (b < c)
				{
					return b;
				}
				else if (a < c)
				{
					return c;
				}
				else
				{
					return a;
				}
			}
			else if (a < c)
			{
				return a;
			}
			else if (b < c)
			{
				return c;
			}
			else
			{
				return b;
			}
		}

		template <typename RandomAccessIterator, typename T> // 快速排序 -- 分割
		RandomAccessIterator __unguraded_partition(RandomAccessIterator first, RandomAccessIterator last, T pivot)
		{
			// 分割的结果最终是：以 piovt 为节点，有如下事实
			// a、节点 pivot 处于正确的位置
			// b、节点 pivot 左边的元素均小于等于 pivot
			// c、节点 pivot 右边的节点均大于等于 pivot
			// d、返回节点 pivot 右边的第一个位置

			while (true)
			{
				while (pivot > *first)
				{
					++first;
				}

				--last;
				while (*last > pivot)
				{
					--last;
				}

				if (!(last > first))
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



		// 如下若干函数，是直接服务于 sort

		template <typename RandomAccessIterator, typename T> //  sort 第二部分 辅助函数：调用 无边界检查的 插入排序
		inline void __ungurded_insertion_sort_aux(RandomAccessIterator first, RandomAccessIterator last, T*)
		{
			for (RandomAccessIterator i = first; i != last; ++i)
			{
				__unguarded_linear_insert(i, _cove_type(*i, T));
			}
		}

		template <typename RandomAccessIterator> // sort 第二部分 辅助函数：调用 插入排序
		inline void __ungurded_insertion_sort(RandomAccessIterator first, RandomAccessIterator last)
		{
			__ungurded_insertion_sort_aux(first, last, ::zhang::iterator::value_type(first));
		}

		template <typename RandomAccessIterator> // sort 第一部分 辅助函数：递归过深时，改用 “堆排序”
		inline void partial_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last)
		{
			__partial_sort(first, middle, last, ::zhang::iterator::value_type(first));
		}

		template <typename Size> // sort 第一部分 辅助函数：用于控制分割恶化情况
		inline Size __lg(Size n) // 找出 2^k <= n 的最大值 k
		{
			auto ans = _init_type(0, Size);

			for (; n > 1; n >>= 1)
			{
				++ans;
			}

			return ans;
		}

		/*------------------------------------------------------------------------------------------------*/



		// 如下两个函数，实行了 sort 的 “两步走” 战略

		// sort -- 第二部分：排序，使 “几乎有序” 蜕变到 “完全有序”
		template <typename RandomAccessIterator>
		inline void __final_insertion_sort(RandomAccessIterator first, RandomAccessIterator last)
		{
			// 待排序元素个数是否足够多？
			if (last - first > __stl_threshold)					  // 是
			{
				__insertion_sort(first, first + __stl_threshold); // 对前若干个元素 插入排序
				__ungurded_insertion_sort(first + __stl_threshold,
										  last); // 对剩余元素(剩余元素数量一定少于前面的元素数量) 插入排序(无边界检查)
			}
			else								 // 否
			{
				__insertion_sort(first, last); // 对这些元素 插入排序
			}
		}

		// sort -- 第一部分：排序，使之 “几乎有序”
		template <typename RandomAccessIterator, typename T, typename Size>
		void __introsort_loop(RandomAccessIterator first, RandomAccessIterator last, T*, Size depth_limt)
		{
			// “几乎有序” 的第一个判断标准：需要排序的元素个数足够少，否则视为 “非 ‘几乎有序’ ”
			while (last - first > __stl_threshold)
			{
				if (depth_limt == 0)								 // 递归深度足够深
				{
					namespace_sort::partial_sort(first, last, last); // 此时调用 partial_sort()，实际上调用了 “堆排序”

					return;
				}

				--depth_limt;

				// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
				RandomAccessIterator cut =
					__unguraded_partition(first,
										  last,
										  _cove_type(__median(*first, *(last - 1), *(first + (last - first) / 2)), T));

				// 对右半段 递归sort
				__introsort_loop(cut, last, ::zhang::iterator::value_type(first), depth_limt);

				last = cut;

				// 至此，回到 while 循环，准备对左半段 递归sort
			}
		}

		/*------------------------------------------------------------------------------------------------*/


		// sort -- 开始排序：这是 sort 函数对外的唯一接口
		template <typename RandomAccessIterator>
		inline void sort(RandomAccessIterator first, RandomAccessIterator last)
		{
			if (first < last) // 真实的排序由以下两个函数完成
			{
				// 排序，使之 “几乎有序”
				__introsort_loop(first, last, ::zhang::iterator::value_type(first), __lg(last - first) * 2);

				// 排序，使 “几乎有序” 蜕变到 “完全有序”
				__final_insertion_sort(first, last);
			}
		}
	} // namespace namespace_sort

	// 对外接口
	using namespace_function::for_each;
	using namespace_function::iter_swap;
	using namespace_sort::sort;

} // namespace zhang::algorithms
