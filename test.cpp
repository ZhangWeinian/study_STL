#pragma once

#include "mySTL.h"

int main(void)
{
	vector<int> iv = { 1, 56, 42, 15, 59, 77, 4, 11, 1, 6, 8, 2, 4, 50, 102, 33, 92, 175, 422, 8, 4, 159, 754, 926 };

	zh::sort(iv.begin(), iv.end());

	zh::for_each(iv.begin(), iv.end(), [](const auto& i) { cout << i << ' '; });

	return 0;
}
