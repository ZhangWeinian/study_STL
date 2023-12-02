#pragma once

#include "mySTL.h"


using _STD boolalpha;
using _STD cout;
using _STD cerr;
using _STD cin;
using _STD endl;

using namespace zh;
namespace ranges = ::std::ranges;

int main(void)
{
	std::vector<int> nums { 15,	  56,	4582, 15,	5927, 77,	10,	  4,	110,  101,	6,	  890,	29,	  4,	 50,
							102,  3173, 10,	  92,	175,  777,	277,  422,	8,	  42,	1435, 48,	159,  754,	 9257,
							77,	  6,	456,  852,	741,  5179, 21,	  28,	74,	  73,	55,	  49,	48,	  17,	 592,
							101,  22,	29,	  7145, 159,  456,	8,	  10,	125,  106,	852,  954,	785,  126,	 9637,
							54,	  56,	8272, 9121, 73,	  38,	2247, 18,	2019, 596,	152,  6,	10,	  482,	 59,
							23,	  70,	10,	  305,	99,	  403,	538,  263,	153,  486,	10,	  759,	426,  103,	 180,
							10,	  197,	458,  483,	10,	  591,	895,  651,	255,  11,	95,	  901,	851,  44,	 91,
							7256, 43,	120,  225,	565,  10,	121,  236,	595,  994,	28,	  3947, 17,	  97,	 46,
							31,	  10,	4880, 10,	61,	  94,	88,	  492,	197,  483,	5,	  4734, 68,	  75,	 1563,
							8765, 1575, 56,	  478,	425,  754,	10,	  154,	458,  1252, 458,  78,	45,	  123,	 645,
							10,	  78,	10,	  425,	63,	  452,	725,  489,	23,	  77,	48,	  23,	42,	  17045, 43869,
							10,	  845,	1425, 27,	2740, 10,	4578, 4836, 48,	  748,	4897, 156,	4867, 1540,	 4557,
							869,  4568, 145,  7421, 7412, 1022, 75,	  47,	10,	  4823, 472,  7413, 953,  237,	 4869 };


	// 1、自定义 插入排序
	// zh::insertion_sort(nums.begin(), nums.end());

	// 2、自定义 快速排序
	// zh::quick_sort(nums.begin(), nums.end());

	// 3、自定义 推排序
	// zh::heap_sort(nums.begin(), nums.end());

	// 4、自定义 标准库排序
	// zh::sort(nums.begin(), nums.end());

	// 5、自定义 归并排序
	// zh::merge_sort(nums.begin(), nums.end());

	// 6、标准库排序
	// ranges::sort(nums, ranges::greater());				 // C++20 及以后
	// std::sort(nums.begin(), nums.end(), std::greater()); // C++20 之前

	_STD vector<int> test = { 1, 2, 3 };

	zh::print("{}", typeid("123456").name());
	zh::print();

	zh::print(1);
	zh::print();

	zh::print(test);
	zh::print();

	zh::print("hello");
	zh::print();

	zh::print(test.begin(), test.end());
	zh::print();

	int arr[3] = { 1, 2, 3 };
	zh::print(arr, 3);
	zh::print();

	const char* str = "123455";
	zh::print(str);
	zh::print();

	// 7、自定义 二分查找
	// const auto [begin, end] = zh::equal_range(nums.begin(), nums.end(), 10);

	return 0;
}
