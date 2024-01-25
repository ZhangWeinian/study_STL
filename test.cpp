#pragma once

#include "./mySTL.h"

#include <concepts>
#include <ranges>

using ::std::boolalpha;
using ::std::cerr;
using ::std::cin;
using ::std::cout;
using ::std::endl;

using ::std::string;
using ::std::vector;

using ::zh::print;
using ::zh::println;
using ::zh::operator""_f;

namespace rg = ::std::ranges;
namespace vi = ::std::ranges::views;

int main(int argc, char* argv[])
{
	vector nums1 { 15,	 56,   10,	 4582, 15,	5927, 77,	10,	  12308, 4,		110,  101,	 6,	  890,	29,	   4,
				   50,	 102,  3173, 10,   92,	175,  777,	277,  422,	 8,		42,	  1435,	 48,  10,	159,   754,
				   9257, 77,   6,	 6345, 867, 32,	  321,	9099, 10,	 456,	852,  741,	 10,  5179, 21,	   5462,
				   28,	 74,   73,	 10,   55,	100,  49,	48,	  17,	 592,	101,  22,	 29,  10,	7145,  159,
				   456,	 8,	   10,	 125,  106, 852,  954,	785,  126,	 9637,	10,	  4562,	 78,  10,	425,   63,
				   452,	 10,   725,	 489,  23,	77,	  48,	23,	  42,	 17045, 10,	  43869, 10,  845,	1425,  27,
				   2740, 10,   4578, 4836, 48,	748,  8965, 10,	  4897,	 156,	4867, 1540,	 10,  4557, 869,   4568,
				   145,	 7421, 7412, 1022, 75,	47,	  10,	4823, 472,	 7413,	953,  10,	 237, 4869, 32767, 1 };

	return 0;
}
