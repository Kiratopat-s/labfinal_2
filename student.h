#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;

template <class T>
class MySort:public Sort<T> {
public:
	void sort(T data[], int size);
};

template <class T>
class MySearch{
public:
	int search(T data[], int size, char *key);
};


template <class T>
void MySort<T>::sort(T data[],int size){
	int i, j;
	for (i = 0; i<(size - 1); i++) {
		for (j = size - 1; j>i; j--) {
			if (data[j] < data[j - 1])
				Sort<T>::swap(&data[j], &data[j - 1]);
			if (Sort<T>::stop) {
				Sort<T>::showData(data, size, i, j);
				cin.get();
			}
		}
	}
}


template <class T>
int MySearch<T>::search(T data[],int size, char *key) {
	int i;
	for (i = 0; i < size; i++)
		if (data[i] == key) return i;
	return -1; // Not found
}
