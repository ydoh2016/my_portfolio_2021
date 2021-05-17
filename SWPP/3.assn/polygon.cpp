#include <cstdint>
#include <vector>
#include <iostream>
using namespace std;

uint64_t area(vector<pair<int, int>> &points) {
  int64_t total = 0;
  uint64_t ret = 0;
  int64_t toadd = 0;
  size_t n = points.size();
  int odd = 0;
  int big = 0;
  int small = 0;

  for (unsigned i = 0; i < n; i++) {
    unsigned j = (i + 1) % n;
    int64_t x_i = points[i].first;
    int64_t y_i = points[i].second;
    int64_t x_j = points[j].first;
    int64_t y_j = points[j].second;
    toadd = x_i * (y_j - y_i) - y_i * (x_j - x_i);
    odd += toadd % 2;
    toadd = toadd / 2;
    total += toadd;
    /* Round the number which is too big or too small */
    if(total > 4500000000000000000)
    {
      big++;
      total -= 4500000000000000000;
    }
    if(total < -4500000000000000000)
    {
      small--;
      total += 4500000000000000000;
    }
  }
  /* if total is negative, it must have been rounded to smaller */
  /* there is no risk of unsigned overflow                      */
  if(total < 0)
  {
    for(unsigned i = big + small; i != 0;)
    {
      if(i > 0)
      {
        ret += 4500000000000000000;
        i--;
      }
      else
      {
        ret -= 4500000000000000000;
        i++;
      }
    }
    ret += total + odd / 2;
    return ret;
  }
  else
  {
    ret += total + odd / 2;
    for(unsigned i = big + small; i != 0;)
    {
      if(i > 0)
      {
        ret += 4500000000000000000;
        i--;
      }
      else
      {
        ret -= 4500000000000000000;
        i++;
      }
    }
    return ret;   
  }
}
