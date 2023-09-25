#pragma once

#include "mySTL.h"

#include <array>
#include <vector>

using _STD vector;
using _STD array;

int main(void)
{
	vector<int> iv = { 1,  56,	42, 15, 59,	 77,  4,  11, 1, 6,	  8,   2,	4,	50, 102, 33, 92, 175, 77,
					   77, 422, 8,	4,	159, 754, 92, 77, 6, 456, 852, 741, 59, 21, 28,	 74, 73, 55,  49 };

	zh::merge_sort(iv.begin(), iv.end());

	auto [begin, end] = zh::make_pair(iv.begin(), iv.end());

	zh::for_each(begin, end, [](const auto& i) { print(i); });

	return 0;
}
