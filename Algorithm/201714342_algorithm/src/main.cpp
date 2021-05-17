#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#include "select.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: ./program <input file>" << endl;
    return EXIT_FAILURE;
  }

  int n, i;
  vector<int> arr;
  ifstream input(argv[1]);

  // Read input.
  if (input.is_open()) {
    int a;
    input >> n >> i;
    for (int idx = 0; idx < n; ++idx) {
      input >> a;
      arr.push_back(a);
    }
    input.close();
  } else {
    cerr << "Failed to read input." << endl;
    return EXIT_FAILURE;
  }

  // Select i-th element (randomized select).
  vector<int> rarr;
  for(int i = 0; i < arr.size(); i++)
  {
    rarr.push_back(arr[i]);
  }
  clock_t x_start = clock();
  int x = randomized_select(rarr, 0, rarr.size() - 1, i);
  clock_t x_finish = clock();

  // Select i-th element (deterministic select).
  vector<int> darr;
  for(int i = 0; i < arr.size(); i++)
  {
    darr.push_back(arr[i]);
  }
  clock_t y_start = clock();
  int y = linear_select(darr, 0, darr.size() - 1, i);
  clock_t y_finish = clock();

  // Check correctness.
  bool x_correct = check(arr, i, x);
  bool y_correct = check(arr, i, y);

  cout << "randomized select    : " << x << " ("
       << (x_correct ? "CORRECT" : "WRONG") << ") (elapsed time: "
       << static_cast<double>(x_finish - x_start) / CLOCKS_PER_SEC << " sec)"
       << endl;
  cout << "deterministic select : " << y << " ("
       << (y_correct ? "CORRECT" : "WRONG") << ") (elapsed time: "
       << static_cast<double>(y_finish - y_start) / CLOCKS_PER_SEC << " sec)"
       << endl;

  return EXIT_SUCCESS;
}
