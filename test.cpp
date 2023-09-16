#include <vector>

/* for myself fun */
#include "01_allocator.hpp"

int main(void)
{
	vector<int, zhang::allocator<int>> iv = { 0, 1, 2, 3, 4 };

	for (const auto& i: iv)
	{
		cout << i << ' ';
	}

	return 0;
}
