#pragma once

#define __USE_ZH__

#include "msvcSTL.h"
#include "mySTL.h"


#ifdef __USE_ZH__

int main(void)
{
	vector<int> nums = { 1,	 56,  42, 15, 59,  77,	4,	11, 1, 6,	8,	 2,	  4,  50, 102, 33, 92, 175, 77,
						 77, 422, 8,  4,  159, 754, 92, 77, 6, 456, 852, 741, 59, 21, 28,  74, 73, 55,	49 };

	zh::merge_sort(nums.begin(), nums.end());

	std::for_each(nums.begin(), nums.end(), [](const auto& i) { cout << i << '\t'; });

	return 0;
}

#else

using namespace std;
using namespace std::placeholders;

int main(void)
{
	vector<int> nums = { 1,	 56,  42, 15, 59,  77,	4,	11, 1, 6,	8,	 2,	  4,  50, 102, 33, 92, 175, 77,
						 77, 422, 8,  4,  159, 754, 92, 77, 6, 456, 852, 741, 59, 21, 28,  74, 73, 55,	49 };

	ranges::sort(nums.begin(), nums.end(), greater<int>());

	ranges::for_each(nums.begin(), nums.end(), [](const auto& i) { cout << i << '\t'; });

	return 0;
}

#endif // __USE_ZH__
