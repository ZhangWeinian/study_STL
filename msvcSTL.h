#pragma once

#include "00_basicFile.h"
#include <string_view>

#include <cstdio>
#include <format>
#include <functional>
#include <string>
#include <vector>


using _STD string;
using _STD vector;

namespace ranges = ::std::ranges;

// 自定义的 print() 函数，简化使用 cout 输出 format()
template <typename... Args>
void print(const ::std::string_view fmt_str, Args&&... args)
{
	fputs((::std::vformat(fmt_str, ::std::make_format_args(args...))).c_str(), stdout);
}
