#pragma once

#include "msvcSTL.h"
#include "mySTL.h"

int main(void)
{
	vector<int> nums { 1,	56,	 42,  15,  59,	77,	 4,	  11,  1,	6,	 8,	  2,   4,	50,	 102, 33,  92,	175,
					   77,	77,	 422, 8,   4,	159, 754, 92,  77,	6,	 456, 852, 741, 59,	 21,  28,  74,	73,
					   55,	49,	 48,  1,   592, 101, 22,  29,  75,	159, 456, 8,   125, 106, 852, 954, 785, 126,
					   963, 54,	 56,  82,  91,	73,	 38,  27,  18,	29,	 596, 152, 6,	482, 59,  23,  70,	305,
					   99,	403, 5,	  263, 153, 486, 759, 426, 103, 180, 197, 48,  483, 591, 895, 651, 2,	11 };

	zh::merge_sort(nums.begin(), nums.end());

	// ranges::sort(nums);

	auto co = 1;
	ranges::for_each(nums,
					 [&co](const auto& i)
					 {
						 cout << i << '\t';

						 if ((co++ % 10) == 0)
						 {
							 cout << endl;
						 }
					 });

	auto test1 = zh::pair(1, 2.);
	auto test2 = zh::pair(1, 2.);

	/*zh::swap(test1, test2);
	cout << boolalpha << (test1 == test2) << endl;*/

	return 0;
}
