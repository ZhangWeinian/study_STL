#pragma once

#include "mySTL.h"


using _STD boolalpha;
using _STD cout;
using _STD cerr;
using _STD cin;
using _STD endl;

using namespace zh;
namespace ranges = ::std::ranges;

int main(int argc, char* argv[])
{
	std::vector<int> nums { 15,	 56,   4582, 15,   5927, 77,   10,	 4,	   110, 101,  6,	890,  29,	4,	   50,
							102, 3173, 10,	 92,   175,	 777,  277,	 422,  8,	42,	  1435, 48,	  159,	754,   9257,
							77,	 6,	   456,	 852,  741,	 5179, 21,	 28,   74,	73,	  55,	49,	  48,	17,	   592,
							101, 22,   29,	 7145, 159,	 456,  8,	 10,   125, 106,  852,	954,  785,	126,   9637,
							10,	 78,   10,	 425,  63,	 452,  725,	 489,  23,	77,	  48,	23,	  42,	17045, 43869,
							10,	 845,  1425, 27,   2740, 10,   4578, 4836, 48,	748,  4897, 156,  4867, 1540,  4557,
							869, 4568, 145,	 7421, 7412, 1022, 75,	 47,   10,	4823, 472,	7413, 953,	237,   4869 };


	std::vector<int> test = nums;

	// 1、自定义 插入排序
	zh::insertion_sort(test);
	zh::print("以下是 插入排序 zh::insertion_sort() 结果\n");
	zh::println(test);
	zh::print("\n\n\n");

	// 2、自定义 快速排序
	test = nums;
	zh::quick_sort(test);
	zh::print("以下是 快速排序 zh::quick_sort() 结果\n");
	zh::println(test);
	zh::print("\n\n\n");

	// 3、自定义 推排序
	test = nums;
	zh::heap_sort(test);
	zh::print("以下是 堆排序 zh::heap_sort() 结果\n");
	zh::println(test);
	zh::print("\n\n\n");

	// 4、自定义 标准库排序
	test = nums;
	zh::sort(test);
	zh::print("以下是 自定义_仿标准库排序 zh::sort() 结果\n");
	zh::println(test);
	zh::print("\n\n\n");

	// 5、自定义 归并排序
	test = nums;
	zh::merge_sort(test);
	zh::print("以下是 归并排序 zh::merge_sort() 结果\n");
	zh::println(test);
	zh::print("\n\n\n");

	// 6、标准库排序
	test = nums;
	ranges::sort(test);
	zh::print("以下是 标准库排序 std::ranges::sort() 结果\n");
	zh::println(test);
	zh::print("\n\n\n");

	// 7、自定义 二分查找
	ranges::sort(test);
	const auto& [my_begin, my_end]	 = zh::equal_range(test, 10);
	const auto& [stl_begin, stl_end] = ranges::equal_range(test, 10);

	zh::print(
		"以下是 自定义_二分查找 zh::equal_range() 结果 与 标准库_二分查找 std::ranges::equal_range() 结果 的对比\n");
	zh::println("zh::equal_range() \t\t起始位置是{}、\t长度是{}\nstd::ranges::equal_range() \t起始位置是{}、\t长度是{}",
				my_begin - test.begin(),
				my_end - my_begin,
				stl_begin - test.begin(),
				stl_end - stl_begin);

	return 0;
}
