#include "00_basicFile.h"

// 此处实现若干 简单函数
namespace zhang
{
	/* function iter_swap */
	template <typename ForwardIterator1, typename ForwardIterator2, typename T>
	inline void __iter_swap(ForwardIterator1 a, ForwardIterator2 b, T*)
	{
		T tmp = *a;
		*a	  = *b;
		*b	  = tmp;
	}

	template <typename ForwardIterator1, typename ForwardIterator2>
	inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b)
	{
		__iter_swap(a, b, value_type(a));
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
} // namespace zhang

// 此处仅实现一个 sort
namespace zhang
{
	// functio sort
	// 插入排序--辅助函数 2 （这是排序的第三步）
	template <typename RandomAccessIterator, typename T>
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

	// 插入排序--辅助函数 1 （这是排序的第二步）
	template <typename RandomAccessIterator, typename T>
	inline void __linear_insert(RandomAccessIterator first, RandomAccessIterator last, T*)
	{
		T value = *last;

		if (*first > value)
		{
			_STD copy_backward(first, last, last + 1);
			*first = value;
		}
		else
		{
			__unguarded_linear_insert(last, value);
		}
	}

	// 插入排序 （这是排序的第一步）
	template <typename RandomAccessIterator>
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
				__linear_insert(first, i, value_type(first));
			}
		}
	}

	/* function partial_sort */
	template <typename RandomAccessIterator, typename T>
	inline void __partial_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, T*)
	{
		make_heap(first, middle);

		for (RandomAccessIterator i = middle; i < last; ++i)
		{
			if (*first > *i)
			{
				__pop_heap(first, middle, i, _cove_type(*i, T), distance_type(first));
			}

			sort_heap(first, middle);
		}
	}

	// 这实际上可以看作一个 “堆排序”
	template <typename RandomAccessIterator>
	inline void partial_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last)
	{
		__partial_sort(first, middle, last, value_type(first));
	}

	// 快速排序 -- 辅助函数：返回三点中值
	template <typename T>
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

	// 快速排序 -- 分割
	// 分割的结果最终是：以 piovt 为节点，有如下事实
	// a、节点 pivot 处于正确的位置
	// b、节点 pivot 左边的元素均小于等于 pivot
	// c、节点 pivot 右边的节点均大于等于 pivot
	// d、返回节点 pivot 右边的第一个位置
	template <typename RandomAccessIterator, typename T>
	RandomAccessIterator __unguraded_partition(RandomAccessIterator first, RandomAccessIterator last, T pivot)
	{
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
				iter_swap(first, last);
				++first;
			}
		}
	}

	// sort -- 第一步
	// 排序，使之 “几乎有序”
	template <typename RandomAccessIterator, typename T, typename Size>
	void __introsort_loop(RandomAccessIterator first, RandomAccessIterator last, T*, Size depth_limt)
	{
		// “几乎有序” 的第一个判断标准：需要排序的元素个数足够少，否则视为 “非 ‘几乎有序’ ”
		while (last - first > __stl_threshold)
		{
			if (depth_limt == 0)				 // 递归深度足够深
			{
				partial_sort(first, last, last); // 此时调用 partial_sort()，实际上调用了 “堆排序”

				return;
			}

			--depth_limt;

			// “非 ‘几乎有序’ ” 时，首先调用 快排 -- 分割
			RandomAccessIterator cut =
				__unguraded_partition(first,
									  last,
									  _cove_type(__median(*first, *(last - 1), *(first + (last - first) / 2)), T));

			// 对右半段 递归sort
			__introsort_loop(cut, last, value_type(first), depth_limt);

			last = cut;

			// 至此，回到 while 循环，准备对左半段 递归sort
		}
	}

	// sort -- 辅助函数
	template <typename RandomAccessIterator, typename T>
	inline void __ungurded_insertion_sort_aux(RandomAccessIterator first, RandomAccessIterator last, T*)
	{
		for (RandomAccessIterator i = first; i != last; ++i)
		{
			__unguarded_linear_insert(i, _cove_type(*i, T));
		}
	}

	// sort -- 辅助函数
	template <typename RandomAccessIterator>
	inline void __ungurded_insertion_sort(RandomAccessIterator first, RandomAccessIterator last)
	{
		__ungurded_insertion_sort_aux(first, last value_type(first));
	}

	// sort -- 辅助函数
	// 排序，使 “几乎有序” 蜕变到 “完全有序”
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

	// sort -- 辅助函数
	// __lg() 用于控制分割恶化情况
	// 找出 2^k <= n 的最大值 k
	template <typename Size>
	inline Size __lg(Size n)
	{
		auto ans = _init_type(0, Size);

		for (; n > 1; n >>= 1)
		{
			++ans;
		}

		return ans;
	}

	// sort
	// 开始排序
	// 这是 sort 函数对外的唯一接口
	template <typename RandomAccessIterator>
	inline void sort(RandomAccessIterator first, RandomAccessIterator last)
	{
		if (first < last) // 真实的排序由以下两个函数完成
		{
			// 排序，使之 “几乎有序”
			__introsort_loop(first, last, value_type(first), __lg(last - first) * 2);

			// 排序，使 “几乎有序” 蜕变到 “完全有序”
			__final_insertion_sort(first, last);
		}
	}
} // namespace zhang
