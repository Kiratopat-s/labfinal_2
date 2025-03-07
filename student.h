#include <iostream>
#include <algorithm>
#include <cctype>
using namespace std;

template <class T>
class MySort : public Sort<T>
{
public:
    void sort(T data[], int size) override;
};

template <class T>
class MySearch
{
public:
    int search(T data[], int size, char *key);
};

template <class T>
void MySort<T>::sort(T data[], int size)
{
    std::sort(data, data + size);
}

template <class T>
int MySearch<T>::search(T data[], int size, char *key)
{
    char lowercaseKey[MAXSTRLEN];
    int i;
    for (i = 0; key[i] != '\0' && i < MAXSTRLEN - 1; ++i)
    {
        lowercaseKey[i] = tolower(key[i]);
    }
    lowercaseKey[i] = '\0';

    Str tempKey(lowercaseKey);

    int low = 0;
    int high = size - 1;

    while (low <= high)
    {
        int mid = low + (high - low) / 2;
        if (data[mid] == tempKey)
        {
            return mid;
        }
        else if (data[mid] > tempKey)
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }

    return -1;
}