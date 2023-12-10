#pragma once

#include "mySTL.h"

using zh::print;
using zh::println;

namespace ranges = ::std::ranges;

int main(int argc, char* argv[])
{
	std::vector test { 15,	 56,   10,	 4582, 15,	  5927, 77,	   10,	12308, 4,	 110,  101,	 6,	   890,	 29,   4,
					   50,	 102,  3173, 10,   92,	  175,	777,   277, 422,   8,	 42,   1435, 48,   10,	 159,  754,
					   9257, 77,   6,	 10,   456,	  852,	741,   10,	5179,  21,	 5462, 28,	 74,   73,	 10,   55,
					   100,	 49,   48,	 17,   592,	  101,	22,	   29,	10,	   7145, 159,  456,	 8,	   10,	 125,  106,
					   852,	 954,  785,	 126,  9637,  10,	4562,  78,	10,	   425,	 63,   452,	 10,   725,	 489,  23,
					   77,	 48,   23,	 42,   17045, 10,	43869, 10,	845,   1425, 27,   2740, 10,   4578, 4836, 48,
					   748,	 8965, 10,	 4897, 156,	  4867, 1540,  10,	4557,  869,	 4568, 145,	 7421, 7412, 1022, 75,
					   47,	 10,   4823, 472,  7413,  953,	10,	   237, 4869 };

	auto& arr1 = test;
	auto& arr2 = test;


	// 使用自定义的 equal_range()
	zh::sort(arr1);
	const auto& [begin1, end1] { zh::equal_range(arr1, 10) };


	// 使用标准库中的 std::ranges::equal_range()
	ranges::sort(arr2);
	const auto& [begin2, end2] { ranges::equal_range(arr2, 10) };


	// 比较
	const auto& first1 { begin1 - arr1.begin() };
	const auto& len1 { end1 - begin2 };
	println("起始位置：{}\t子区间长度：{}", first1, len1);
	println(arr1.begin() + first1 - 1, arr1.begin() + first1 + len1 + 1);
	print();

	const auto& first2 { begin2 - arr2.begin() };
	const auto& len2 { end2 - begin2 };
	println("起始位置：{}\t子区间长度：{}", first2, len2);
	println(arr2.begin() + first2 - 1, arr2.begin() + first2 + len2 + 1);
	print();


	system("pause");

	return 0;
}
