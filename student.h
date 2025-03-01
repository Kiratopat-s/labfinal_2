#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <future>
using namespace std;

template <class T>
class MySort:public Sort<T> {
public:
	void sort(T data[], int size);
private:
	void parallelMergeSort(T data[], int left, int right, int depth);
	void merge(T data[], int left, int mid, int right);
};

template <class T>
class MySearch {
public:
	int search(T data[], int size, char *key);
private:
	int parallelBinarySearch(T data[], int left, int right, const T& key, int depth);
};

template <class T>
void MySort<T>::sort(T data[], int size) {
	int maxDepth = std::thread::hardware_concurrency();
	parallelMergeSort(data, 0, size - 1, maxDepth);
}

template <class T>
void MySort<T>::parallelMergeSort(T data[], int left, int right, int depth) {
	if (left >= right) return;

	if (depth <= 0) {
		std::sort(data + left, data + right + 1);
	} else {
		int mid = left + (right - left) / 2;
		std::thread leftThread(&MySort::parallelMergeSort, this, data, left, mid, depth - 1);
		std::thread rightThread(&MySort::parallelMergeSort, this, data, mid + 1, right, depth - 1);
		leftThread.join();
		rightThread.join();
		merge(data, left, mid, right);
	}
}

template <class T>
void MySort<T>::merge(T data[], int left, int mid, int right) {
	int n1 = mid - left + 1;
	int n2 = right - mid;

	std::vector<T> leftArray(n1);
	std::vector<T> rightArray(n2);

	for (int i = 0; i < n1; i++)
		leftArray[i] = data[left + i];
	for (int i = 0; i < n2; i++)
		rightArray[i] = data[mid + 1 + i];

	int i = 0, j = 0, k = left;
	while (i < n1 && j < n2) {
		if (leftArray[i] <= rightArray[j]) {
			data[k] = leftArray[i];
			i++;
		} else {
			data[k] = rightArray[j];
			j++;
		}
		k++;
	}

	while (i < n1) {
		data[k] = leftArray[i];
		i++;
		k++;
	}

	while (j < n2) {
		data[k] = rightArray[j];
		j++;
		k++;
	}
}

template <class T>
int MySearch<T>::search(T data[], int size, char *key) {
	T keyStr(key);
	int maxDepth = std::thread::hardware_concurrency();
	return parallelBinarySearch(data, 0, size - 1, keyStr, maxDepth);
}

template <class T>
int MySearch<T>::parallelBinarySearch(T data[], int left, int right, const T& key, int depth) {
	if (left > right) return -1;

	if (depth <= 0) {
		while (left <= right) {
			int mid = left + (right - left) / 2;
			if (data[mid] == key) return mid;
			if (data[mid] < key) left = mid + 1;
			else right = mid - 1;
		}
		return -1;
	} else {
		int mid = left + (right - left) / 2;
		if (data[mid] == key) return mid;

		std::future<int> leftResult = std::async(&MySearch::parallelBinarySearch, this, data, left, mid - 1, key, depth - 1);
		std::future<int> rightResult = std::async(&MySearch::parallelBinarySearch, this, data, mid + 1, right, key, depth - 1);

		int leftIndex = leftResult.get();
		if (leftIndex != -1) return leftIndex;

		return rightResult.get();
	}
}
