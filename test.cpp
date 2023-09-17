#pragma once

#include "00_basicFile.hpp"

int main(void)
{
	vector<int> iv = { 0, 1, 2, 3, 4 };

	_STD copy_backward(iv.begin(), iv.begin() + 2, iv.begin() + 3);

	for (const auto& i: iv)
	{
		cout << i << ' ';
	}

	return 0;
}
