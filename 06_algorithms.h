#include "00_basicFile.h"

// 此处实现若干函数
namespace zhang::algorithms
{
	// 此处实现若干简单函数
	namespace namespace_function
	{
		/* function swap() */
		template <typename T>
		inline void swap(T& a, T& b) noexcept
		{
			T tmp = a;
			a	  = b;
			b	  = tmp;
		}

		/* function iter_swap() -- 辅助函数 */
		template <typename ForwardIterator1, typename ForwardIterator2, typename T>
		inline void __swap(ForwardIterator1 a, ForwardIterator2 b, T*)
		{
			T tmp = *a;
			*a	  = *b;
			*b	  = tmp;
		}

		/* function iter_swap() */
		template <typename ForwardIterator1, typename ForwardIterator2>
		inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b)
		{
			__swap(a, b, ::zhang::iterator::value_type(a));
		}

		/* function for_each() */
		template <typename InputIterator, typename Function>
		Function for_each(InputIterator first, InputIterator last, Function f)
		{
			for (; first != last; ++first)
			{
				f(*first);
			}

			return f;
		}

		//

	} // namespace namespace_function

	// 此处实现 lower_bound()、upper_bound()、equal_range()、binary_search()
	namespace namespace_binary_search
	{
		// 此处需要较多的用到跌倒其类型标签，为了使用方便，定义如下宏
#ifndef __ZH_ITERATOR__
	#define __ZH_ITERATOR__ ::zhang::iterator::namespace_iterator::
#endif // !__ZH_ITERATOR__


		/* upper_bound() -- 辅助函数 */
		template <typename RandomAccessIterator, typename T, typename Distance> // 针对 RandomAccessIterator 的版本
		inline RandomAccessIterator __upper_bound(RandomAccessIterator first,
												  RandomAccessIterator last,
												  const T&			   value,
												  Distance*,
												  __ZH_ITERATOR__ random_access_iterator_tag)
		{
			Distance			 len	= last - first;
			Distance			 half	= len;
			RandomAccessIterator middle = first;

			while (len > 0)
			{
				half   = len >> 1;
				middle = first + half;

				if (value < *middle)
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
		template <typename ForwardIterator, typename T, typename Distance> // 针对 ForwardIterator 的版本
		inline ForwardIterator __upper_bound(ForwardIterator first,
											 ForwardIterator last,
											 const T&		 value,
											 Distance*,
											 __ZH_ITERATOR__ forward_iterator_tag)
		{
			Distance len  = 0;
			Distance half = 0;

			::zhang::iterator::distance(first, last, len);

			ForwardIterator middle = first;

			while (len > 0)
			{
				half = len >> 1;

				middle = first;
				::zhang::iterator::advance(middle, half);

				if (value < *middle)
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

		/* upper_bound() */
		template <typename ForwardIterator, typename T>
		inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const T& value)
		{
			return __upper_bound(first,
								 last,
								 value,
								 ::zhang::iterator::distance_type(first),
								 ::zhang::iterator::iterator_category(first));
		}

		/* function lower_bound() -- 辅助函数 */
		template <typename RandomAccessIterator, typename T, typename Distance> // 针对 RandomAccessIterator 的版本
		inline RandomAccessIterator __lower_bound(RandomAccessIterator first,
												  RandomAccessIterator last,
												  const T&			   value,
												  Distance*,
												  __ZH_ITERATOR__ random_access_iterator_tag)
		{
			Distance len  = last - first;
			Distance half = len;

			RandomAccessIterator middle = first;

			while (len > 0)
			{
				half   = len >> 1;
				middle = first + half;

				if (*middle < value)
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
		template <typename ForwardIterator, typename T, typename Distance> // 针对 ForwardIterator 的版本
		inline ForwardIterator __lower_bound(ForwardIterator first,
											 ForwardIterator last,
											 const T&		 value,
											 Distance*,
											 __ZH_ITERATOR__ forward_iterator_tag)
		{
			Distance len  = 0;
			Distance half = 0;
			::zhang::iterator::distance(first, last, len);

			ForwardIterator middle = first;

			while (len > 0)
			{
				half = len >> 1;

				middle = first;
				::zhang::iterator::advance(middle, half);

				if (*middle < value)
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

		/* function lower_bound() */
		template <typename ForwardIterator, typename T>
		inline ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const T& value)
		{
			return __lower_bound(first,
								 last,
								 value,
								 ::zhang::iterator::distance_type(first),
								 ::zhang::iterator::iterator_category(first));
		}

		/* function equal_range() -- 辅助函数 */
		template <typename RandomAccessIterator, typename T, typename Distance> // 针对 RandomAccessIterator 的版本
		inline ::zhang::without_book::pair<RandomAccessIterator, RandomAccessIterator>
			__equal_range(RandomAccessIterator first,
						  RandomAccessIterator last,
						  const T&			   value,
						  Distance*,
						  __ZH_ITERATOR__ random_access_iterator_tag)
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

				if (*middle < value)	 // 如果 中央元素 < 指定值
				{
					first  = middle + 1; // 将区间缩小（移至后半段），以提高效率
					len	  -= (half + 1);
				}
				else if (*middle > value)					   // 如果 中央元素 > 指定值
				{
					len = half;								   // 将区间缩小（移至前半段），以提高效率
				}
				else										   // 如果 中央元素 == 指定值
				{
					left  = lower_bound(first, middle, value); // 在前半段寻找 lower_bound
					right = upper_bound(++middle, first + len, value); // 在后半段寻找 lower_bound

					return ::zhang::without_book::namespace_pair::pair<RandomAccessIterator, RandomAccessIterator>(
						left,
						right);
				}
			}

			// 整个区间内没有匹配值，则返回一对迭代器--指向第一个 大于指定值 的元素
			return ::zhang::without_book::namespace_pair::pair<RandomAccessIterator, RandomAccessIterator>(first,
																										   first);
		}

		/* function equal_range() -- 辅助函数 */
		template <typename ForwardIterator, typename T, typename Distance> // 针对 ForwardIterator 的版本
		inline ::zhang::without_book::pair<ForwardIterator, ForwardIterator>
			__equal_range(ForwardIterator first,
						  ForwardIterator last,
						  const T&		  value,
						  Distance*,
						  __ZH_ITERATOR__ forward_iterator_tag)
		{
			Distance len  = 0;
			Distance half = 0;

			::zhang::iterator::distance(first, last, len);

			ForwardIterator left   = first;
			ForwardIterator middle = first;
			ForwardIterator right  = first;

			while (len > 0)
			{
				half = len >> 1;

				middle = first;
				::zhang::iterator::advance(middle, half);

				if (*middle < value)
				{
					first == middle;
					++first;
					len -= (half + 1);
				}
				else if (*middle > value)
				{
					len = half;
				}
				else
				{
					left = lower_bound(first, middle, value);

					::zhang::iterator::advance(first, len);

					right = upper_bound(++middle, first, value);

					return ::zhang::without_book::namespace_pair::pair<ForwardIterator, ForwardIterator>(left, right);
				}
			}

			return ::zhang::without_book::namespace_pair::pair<ForwardIterator, ForwardIterator>(first, first);
		}

		/* function equal_range() */
		template <typename ForwardIterator, typename T>
		inline ::zhang::without_book::pair<ForwardIterator, ForwardIterator>
			equal_range(ForwardIterator first, ForwardIterator last, const T& value)
		{
			return __equal_range(first,
								 last,
								 value,
								 ::zhang::iterator::distance_type(first),
								 ::zhang::iterator::iterator_category(first));
		}

		/* function binary_search() */
		template <typename ForwardIterator, typename T>
		inline bool binary_search(ForwardIterator first, ForwardIterator last, const T& value)
		{
			ForwardIterator i = lower_bound(first, last, value);

			return i != last && !(value < *i);
		}

		// 离开此命名空间，取消定义此宏，避免在它处产生干扰
#ifdef __ZH_ITERATOR__
	#undef __ZH_ITERATOR__
#endif // __ZH_ITERATOR__

	}  // namespace namespace_binary_search

	// 此处实现 sort()、quick_sort()、insertion_sort()、merge_sort()
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
					::zhang::sequence_containers::namespace_heap::__pop_heap(first,
																			 middle,
																			 i,
																			 _cove_type(*i, T),
																			 ::zhang::iterator::distance_type(first));
				}
			}

			::zhang::sequence_containers::sort_heap(first, middle);
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



		// 如下若干函数，直接服务于 sort

		template <typename RandomAccessIterator,
				  typename T> //  sort 第二部分 辅助函数：调用 无边界检查的 插入排序
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
			// “几乎有序” 的判断标准：需要排序的元素个数足够少，否则视为 “非 ‘几乎有序’ ”
			while (last - first > __stl_threshold)
			{
				if (depth_limt == 0) // 递归深度足够深
				{
					namespace_sort::partial_sort(first,
												 last,
												 last); // 此时调用 partial_sort()，实际上调用了一个 “堆排序”

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



		// sort -- SGI STL 标准函数
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

		/*-----------------------------------------------------------------------------------------------*/



		// 此处封装 sort 内部完整的 插入排序
		namespace insertion_sort
		{
			template <typename RandomAccessIterator> // 插入排序 （这是排序的第一步）
			inline void insertion_sort(RandomAccessIterator first, RandomAccessIterator last)
			{
				::zhang::algorithms::namespace_sort::__insertion_sort(first, last);
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

			template <typename BidirectionalIterator>
			inline void merge_sort(BidirectionalIterator first, BidirectionalIterator last)
			{
				typename ::zhang::iterator::iterator_traits<BidirectionalIterator>::difference_type n =
					::zhang::iterator::distance(first, last);

				if (n == 0 || n == 1)
				{
					return;
				}

				BidirectionalIterator mid = first + n / 2;

				merge_sort(first, mid);
				merge_sort(mid, last);

				// HACK: 以期实现自己的 inplace_merge() ，同时，此前提到插入排序的缺点之一 “借助额外内存” ，就体现在此函数中
				_STD inplace_merge(first, mid, last);
			}

		} // namespace merge_sort

		/*-----------------------------------------------------------------------------------------------*/



		// 此处实现 快速排序
		namespace quick_sort
		{
			template <typename RandomAccessIterator, typename T>
			inline void __quick_sort(RandomAccessIterator first, RandomAccessIterator last, T*)
			{
				while (last - first > 1)
				{
					auto pivot =
						_cove_type(namespace_sort::__median(*first, *(last - 1), *(first + (last - first) / 2)), T);

					RandomAccessIterator cut = namespace_sort::__unguraded_partition(first, last, pivot);

					if ((cut - last) >= (last - cut))
					{
						__quick_sort(cut, last, ::zhang::iterator::value_type(last));
						last = cut;
					}
					else
					{
						__quick_sort(first, cut, ::zhang::iterator::value_type(first));
						first = cut;
					}
				}
			}

			template <typename RandomAccessIterator>
			inline void quick_sort(RandomAccessIterator first, RandomAccessIterator last)
			{
				if (first >= last)
				{
					return;
				}
				__quick_sort(first, last, ::zhang::iterator::value_type(first));
			}

		} // namespace quick_sort

	}	  // namespace namespace_sort

	// 此处实现 copy()
	namespace namespace_copy
	{
		// 此处需要较多的用到跌倒其类型标签，为了使用方便，定义如下宏
#ifndef __ZH_ITERATOR__
	#define __ZH_ITERATOR__ ::zhang::iterator::namespace_iterator::
#endif // !__ZH_ITERATOR__


		// 如下两个函数服务于仿函数 __copy_dispatch() 的两个 偏特化版本

		/* function __copy_t() -- 辅助函数 */
		template <typename T> // 以下版本适用于 “指针所指之对象，具备 trivial assignment operator ”
		inline T* __copy_t(const T* first, const T* last, T* result, __ZH_ITERATOR__ __true_type)
		{
			memmove(result, first, sizeof(T) * (last - first));

			return result + (last - first);
		}

		template <typename T> // 以下版本适用于 “指针所指之对象，具备 non-trivial assignment operator ”
		inline T* __copy_t(const T* first, const T* last, T* result, __ZH_ITERATOR__ __false_type)
		{
			return __copy_d(first, last, result, _cove_type(0, ptrdiff_t*));
		}

		/*-----------------------------------------------------------------------------------------*/



		// 如下三个（实际可以合并两个。单独分出一个的原因是为了其他地方也能用到）函数服务于仿函数 __copy_dispatch() 的 完全泛化版本

		/* function __copy_d() -- 辅助函数 */
		template <typename RandomAccessIterator, typename OutputIterator, typename Distance>
		inline OutputIterator
			__copy_d(RandomAccessIterator first, RandomAccessIterator last, OutputIterator result, Distance*)
		{
			for (Distance i = last - first; i > 0; --i, ++result, ++first) // 以 n 决定循环的次数 -- 速度快
			{
				*result = *first;
			}

			return result;
		}

		/* function __copy() -- 辅助函数 */
		template <typename InputIterator, typename OutputIterator> // InputIterator 版本
		inline OutputIterator
			__copy(InputIterator first, InputIterator last, OutputIterator result, __ZH_ITERATOR__ input_iterator_tag)
		{
			for (; first != last; ++result, ++first) // 以迭代器相同与否，决定循环是否继续 -- 速度慢
			{
				*result = *first;					 // assignment operator
			}
		}

		template <typename RandomAccessIterator, typename OutputIterator> // RandomAccessIterator 版本
		inline OutputIterator __copy(RandomAccessIterator first,
									 RandomAccessIterator last,
									 OutputIterator		  result,
									 __ZH_ITERATOR__	  random_access_iterator_tag)
		{
			// 又划分出一个函数，为的是其他地方也能用到
			return __copy_d(first, last, result, ::zhang::iterator::distance_type(first));
		}

		/*-----------------------------------------------------------------------------------------*/



		// 此仿函数直接服务于 copy()，它分为一个 完全泛化版本 和两个 偏特化版本

		/* function copy() -- 辅助函数 */
		template <typename InputIterator, typename OutputIterator> // 完全泛化版本
		struct __copy_dispatch
		{
			OutputIterator operator()(InputIterator first, InputIterator last, OutputIterator result)
			{
				return __copy(
					first,
					last,
					result,
					::zhang::iterator::iterator_category(
						first)); // 此处兵分两路：根据迭代器种类的不同，调用不同的 __copy() ，为的是不同种类的迭代器所使用的循环条件不同，有快慢之分
			}
		};

		template <typename T> // 偏特化版本1：两个参数都是 T* 指针形式
		struct __copy_dispatch<T*, T*>
		{
			T* operator()(T* first, T* last, T* result)
			{
				using t = typename ::zhang::iterator::__type_traits<T>::has_trivial_assignment_operator;
				return __copy_t(first, last, result, t());
			}
		};

		template <typename T> // 偏特化版本2：第一个参数是 const T* 指针形式，第二个参数是 T* 指针形式
		struct __copy_dispatch<const T*, T*>
		{
			T* operator()(const T* first, const T* last, T* result)
			{
				using t = typename ::zhang::iterator::__type_traits<T>::has_trivial_assignment_operator;
				return __copy_t(first, last, result, t());
			}
		};

		/*-----------------------------------------------------------------------------------------*/



		// 以下三个函数是 copy() 的对外接口

		/* function copy() 重载1 */
		inline char* copy(const char* first,
						  const char* last,
						  char* result) // 针对原生指针(可视为一种特殊的迭代器) const char* ，机型内存直接拷贝操作
		{
			memmove(result, first, last - first);
			return result + (last - first);
		}

		/* function copy() 重载2 */
		inline wchar_t*
			copy(const wchar_t* first,
				 const wchar_t* last,
				 wchar_t* result) // 针对原生指针(可视为一种特殊的迭代器) const wchar_t* ，机型内存直接拷贝操作
		{
			memmove(result, first, sizeof(wchar_t) * (last - first));
			return result + (last - first);
		}

		/* function copy() */
		template <typename InputIterator, typename OutputIterator> // 完全泛化版本
		inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
		{
			return __copy_dispatch<InputIterator, OutputIterator>()(first, last, result);
		}

		// 离开此命名空间，取消定义此宏，避免在它处产生干扰
#ifdef __ZH_ITERATOR__
	#undef __ZH_ITERATOR__
#endif // __ZH_ITERATOR__

	}  // namespace namespace_copy

	/*-----------------------------------------------------------------------------------------*/



	/* 统一的对外接口 */

	using namespace_binary_search::
		binary_search; // 二分查找：是否存在元素，其等于指定元素 -- 存在：返回 true ；不存在：返回 false

	using namespace_binary_search::
		equal_range; // 二分查找：是否存在区间，其中的每一个元素等于指定元素 -- 存在：返回满足条件的最大子区间 ；不存在：返回“假定这个元素存在时，最应该出现的位置”

	using namespace_binary_search::
		lower_bound; // 二分查找：是否存在元素，其等于指定元素 -- 存在：返回其中第一个元素 ；不存在：返回空区间

	using namespace_binary_search::
		upper_bound; // 二分查找：是否存在元素，其等于指定元素 -- 存在：返回其中第一个元素 ；不存在：返回“在不破坏顺序的情况下，可插入指定元素的最后一个位置”

	using namespace_copy::copy;							  // 标准库 copy()
	using namespace_function::for_each;					  // 标准库 for_each()
	using namespace_function::iter_swap;				  // 标准库 iter_swap()
	using namespace_function::swap;						  // 标准库 swap()
	using namespace_sort::sort;							  // 标准库 sort() 排序
	using namespace_sort::insertion_sort::insertion_sort; // 标准库形式的 插入排序
	using namespace_sort::merge_sort::merge_sort;		  // 标准库形式的 归并排序
	using namespace_sort::quick_sort::quick_sort;		  // 标准库形式的 快速排序

} // namespace zhang::algorithms
