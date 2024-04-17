#pragma once

#include "./mySTL.h"

#include <concepts>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

using ::zh::print;
using ::zh::println;
using ::zh::operator""_f;

namespace rg = ::std::ranges;
namespace vi = ::std::ranges::views;

int main(int argc, char* argv[])
{
	vector nums { 1, 3, 5, 6, 8 };

	zh::sort(nums, rg::greater {});

	return 0;
}
