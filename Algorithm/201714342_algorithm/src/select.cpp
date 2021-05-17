#include <iostream>
#include <ctime>
#include "select.hpp"

using namespace std;

/****************************************************************
  README:
    p to r used in operand means [p, r] which includes p and r.
    i.e. arr size of 5 can be represented with (p = 0, r = 4).
    it also should be respected in main.cpp
****************************************************************/

/*part arr from p to r with random pivot from arr*/
int randomized_partition(vector<int> &arr, int p, int r)
{
    /*get random as pivot from rand()*/
    srand(time(NULL));
    int random = rand() % (-p + r + 1);
    int random_idx = p + random;
    /*swap the selected element to the end of the array*/
    swap(arr[random_idx], arr[r]);
    int x = arr[r];
    int i = p - 1;
    /*compare all elements except pivot and put smaller elements left, larger elements right*/
    for(int j = p; j < r; j++)
    {
        if(arr[j] <= x)
        {
            swap(arr[++i], arr[j]);
        }
    }
    /*let pivot in center*/
    swap(arr[++i], arr[r]);
    return i;
}
/*select ith(i > 0) element in arr from p to r*/
int randomized_select(vector<int> &arr, int p, int r, int i)
{
    /*end case of recursive randomized_select : n is 1*/
    if(p == r) return arr[p];
    /*make partition with random element*/
    int q = randomized_partition(arr, p, r);
    int k = q - p + 1;
    /*recursively call randomized_select with partition ith element is in*/
    if(i < k) return randomized_select(arr, p, q - 1, i);
    else if(i == k) return arr[q];
    else return randomized_select(arr, q + 1, r, i - k);
}

/*part arr from p to r with x pivot from arr*/
int deterministic_partition(vector<int> &arr, int p, int r, int x)
{
    int i = p - 1;
    int encountered = 0;
    /*definitely there is x in arr*/
    for(int j = p; j <= r; j++)
    {
        if(arr[j] == x)
        {
            /*swap the selected element to the end of the array*/
            swap(arr[j], arr[r]);
            break;
        }
    }
    /*compare all elements except pivot and put smaller elements left, larger elements right*/
    for(int j = p; j < r; j++)
    {
        if(arr[j] <= x)
        {
            swap(arr[++i], arr[j]);
        }
    }
    swap(arr[++i], arr[r]);
    return i;
}
/*insertion sort arr from p to r*/
int insertion_sort(vector<int> &arr, int p, int r)
{
    int key;
    int j;
    for(int i = p + 1; i < r + 1; i++)
    {
        key = arr[i];
        j = i - 1;
        while(j >= p && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
    return p;
}
/*select ith(i > 0) element in arr from p to r*/
int linear_select(vector<int> &arr, int p, int r, int i)
{
    if(r - p < 5)
    {
        insertion_sort(arr, p, r);
        return arr[p + i - 1];
    }
    vector<vector<int>> subarr; // slice array with n = 5
    vector<int> median; // array of medians from each subarray
    for(int j = p; j <= r;)
    {
        vector<int> tmp;
        for(int k = 0; k < 5; k++)
        {
            if(j > r) break;
            tmp.push_back(arr[j]);
            j++;
        }
        subarr.push_back(tmp);
        /*simultaneously slice array and calculate median*/
        insertion_sort(tmp, 0, tmp.size() - 1);
        median.push_back(tmp[(tmp.size() - 1) / 2]);
    }
    /*m is same with the element in center of the array*/
    int m = linear_select(median, 0, median.size() - 1, (median.size() + 1) / 2);
    /*partition with m*/
    int q = deterministic_partition(arr, p, r, m);
    int k = q - p + 1;
    /*recursively call linear_select with partition ith element is in*/
    if(i < k) return linear_select(arr, p, q - 1, i);
    else if(i == k) return arr[q];
    else return linear_select(arr, q + 1, r, i - k);
}

bool check(vector<int> &arr, int i, int x)
{
    int s = 0; // number of smaller elements than x
    int e = 0; // number of equal elements than x
    int b = 0; // number of larger elemnts than x ( not needed )
    for(int j = 0; j < arr.size(); j++)
    {
        if(arr[j] < x) s++;
        else if(arr[j] == x) e++;
        else b++;
    }
    if(s < i && i <= s + e) return true;
    else return false;
}
