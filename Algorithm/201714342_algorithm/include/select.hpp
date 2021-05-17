#ifndef _SELECT_H_
#define _SELECT_H_

#include <vector>

int randomized_partition(std::vector<int> &, int, int);
int randomized_select(std::vector<int> &, int, int, int);
int linear_select(std::vector<int> &, int, int, int);
int deterministic_partition(std::vector<int> &, int, int, int);
bool check(std::vector<int> &, int, int);

#endif
