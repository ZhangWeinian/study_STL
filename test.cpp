#pragma once

#include "mySTL.h"

int main(void)
{
	vector<int> iv = { 1,  56,	42, 15, 59,	 77,  4,  11, 1, 6,	  8,   2,	4,	50, 102, 33, 92, 175, 77,
					   77, 422, 8,	4,	159, 754, 92, 77, 6, 456, 852, 741, 59, 21, 28,	 74, 73, 55,  49 };

	zh::sort(iv.begin(), iv.end());

	auto [begin, end] = zh::equal_range(iv.begin(), iv.end(), 77);

	zh::for_each(begin, end, [](const auto& i) { cout << i << '\b'; });

	return 0;
}
