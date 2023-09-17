#include <algorithm>

#ifndef _mySort


template <typename Iter>
inline void _insertion_sort(Iter first, Iter last)
{
	if (first >= last)
	{
		return;
	}
	else
	{
		for (Iter i = first + 1; i != last; ++i)
		{
			_liner_insert(first, i, _STD value_type(first));
		}
	}
}

template <typename Iter, typename T>
inline void _liner_insert(Iter first, Iter last, T*)
{
	T value = *last;

	if (*first > value)
	{
		_STD copy_backward(first, last, last + 1);
		*first = value;
	}
	else
	{
		_unguraded_line_insert(last, value);
	}
}

template <typename Iter, typename T>
inline void _unguraded_line_insert(Iter last, T value)
{
	Iter next = last;
	--next;

	while (*next > value)
	{
		*last = *next;
		last = next;
		--next;
	}

	*last = value;
}
#endif // !_mySort
