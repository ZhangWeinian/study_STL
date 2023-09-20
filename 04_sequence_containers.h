#include "00_basicFile.h"

namespace zhang::sequence_containers
{
	/* heap 算法 */
	namespace namespace_heap
	{
		// push_heap
		template <typename RandomAccessIterator, typename Distance, typename T>
		inline void __push_heap(RandomAccessIterator first, Distance holeIndex, Distance topIndex, T value)
		{
			Distance parent = (holeIndex - 1) / 2;

			while (holeIndex > topIndex && *(first + parent) < value)
			{
				*(first + holeIndex) = *(first + parent);
				holeIndex			 = parent;
				parent				 = (holeIndex - 1) / 2;
			}

			*(first + holeIndex) = value;
		}

		template <typename RandomAccessIterator, typename Distance, typename T>
		inline void __push_heap_aux(RandomAccessIterator first, RandomAccessIterator last, Distance*, T*)
		{
			__push_heap(first,
						_cove_type((last - first) - 1, Distance),
						_init_type(0, Distance),
						_cove_type(*(last - 1), T));
		}

		template <typename RandomAccessIterator>
		inline void push_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			__push_heap_aux(first, last, ::zhang::iterator::distance_type(first), ::zhang::iterator::value_type(first));
		}

		// pop_heap
		template <typename RandomAccessIterator, typename Distance, typename T>
		inline void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value)
		{
			Distance topIndex	 = holeIndex;
			Distance secondChild = 2 * holeIndex + 2;

			while (secondChild < len)
			{
				if (*(first + (secondChild - 1)) > *(first + secondChild))
				{
					secondChild--;
				}

				*(first + holeIndex) = *(first + secondChild);
				holeIndex			 = secondChild;
				secondChild			 = 2 * (secondChild + 1);
			}

			if (secondChild == len)
			{
				*(first + holeIndex) = *(first + (secondChild - 1));
			}

			__push_heap(first, holeIndex, topIndex, value);
		}

		template <typename RandomAccessIterator, typename T, typename Distance>
		inline void __pop_heap(RandomAccessIterator first,
							   RandomAccessIterator last,
							   RandomAccessIterator result,
							   T					value,
							   Distance*)
		{
			*result = *first;

			__adjust_heap(first, _init_type(0, Distance), _cove_type(last - first, Distance), value);
		}

		template <typename RandomAccessIterator, typename T>
		inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*)
		{
			__pop_heap(first, last - 1, last - 1, _cove_type(*(last - 1), T), ::zhang::iterator::distance_type(first));
		}

		template <typename RandomAccessIterator>
		inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			__pop_heap_aux(first, last, ::zhang::iterator::value_type(first));
		}

		// sort_heap
		template <typename RandomAccessIterator>
		inline void sort_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			while (last - first > 1)
			{
				::zhang::sequence_containers::namespace_heap::pop_heap(first, last--);
			}
		}

		// make_heap
		template <typename RandomAccessIterator, typename T, typename Distance>
		inline void __make_heap(RandomAccessIterator first, RandomAccessIterator last, T*, Distance*)
		{
			if (last - first < 2)
			{
				return;
			}

			Distance len	= last - first;
			Distance parent = (len - 2) / 2;

			while (true)
			{
				__adjust_heap(first, parent, len, _cove_type(*(first + parent), T));

				if (parent == 0)
				{
					return;
				}

				parent--;
			}
		}

		template <typename RandomAccessIterator>
		inline void make_heap(RandomAccessIterator first, RandomAccessIterator last)
		{
			__make_heap(first, last, ::zhang::iterator::value_type(first), ::zhang::iterator::distance_type(first));
		}

	} // namespace namespace_heap

	// 对外接口
	using namespace_heap::make_heap;
	using namespace_heap::pop_heap;
	using namespace_heap::push_heap;
	using namespace_heap::sort_heap;

} // namespace zhang::sequence_containers
