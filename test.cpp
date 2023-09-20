#pragma once

#include "mySTL.h"

int main(void)
{
	vector<int> iv = { 1, 56, 42, 15, 59, 77, 4, 111, 6, 8, 2, 4 };

	_ZH sort(iv.begin(), iv.end());

	_ZH for_each(iv.begin(),
				 iv.end(),
				 [&iv](const auto& i)
				 {
					 cout << i << ' ';
					 if (i == iv.back())
					 {
						 cout << endl << "work over" << endl;
					 }
				 });

	return 0;
}
