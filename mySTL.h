#pragma once

#include "01_without_book.h"
#include "03_iterator.h"
#include "04_sequence_containers.h"
#include "06_algorithms.h"

namespace zh
{
	// 使用预定义的统一对外接口
	using namespace ::zhang::algorithms;
	using namespace ::zhang::without_book;
	using namespace ::zhang::iterator;
	using namespace ::zhang::sequence_containers;

} // namespace zh

template <typename T>
inline void print(const T& i)
{
	cout << i << ' ';
}
