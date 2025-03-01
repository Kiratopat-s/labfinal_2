#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <algorithm>
#include <array>
using namespace std;

template <class T>
class MySort:public Sort<T> {
public:
	void sort(T data[], int size);
private:
	void parallelMergeSort(T data[], int left, int right, int depth);
	void merge(T data[], int left, int mid, int right);
	void insertionSort(T data[], int left, int right);
	static const int INSERTION_SORT_THRESHOLD = 32; // Threshold for switching to insertion sort
	static const int MAX_THREADS; // Maximum number of threads to create
};

// Define the static constant outside the class
template <class T>
const int MySort<T>::MAX_THREADS = 2;

template <class T>
class MySearch {
public:
	int search(T data[], int size, char *key);
private:
	unordered_map<string, int> hashTable;
	array<pair<string, int>, 16> cache; // Small fixed-size cache
	void buildHashTable(T data[], int size);
	int cacheLookup(const string& key);
	void cacheInsert(const string& key, int value);
};

template <class T>
void MySort<T>::sort(T data[], int size) {
	int maxDepth = min(MAX_THREADS, static_cast<int>(std::thread::hardware_concurrency()));
	parallelMergeSort(data, 0, size - 1, maxDepth);
}

template <class T>
void MySort<T>::parallelMergeSort(T data[], int left, int right, int depth) {
	if (left >= right) return;

	if (right - left + 1 <= INSERTION_SORT_THRESHOLD) {
		insertionSort(data, left, right);
		return;
	}

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
		if (!(leftArray[i] > rightArray[j])) { // Use !(a > b) instead of a <= b
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
void MySort<T>::insertionSort(T data[], int left, int right) {
	for (int i = left + 1; i <= right; i++) {
		T key = data[i];
		int j = i - 1;
		while (j >= left && data[j] > key) {
			data[j + 1] = data[j];
			j--;
		}
		data[j + 1] = key;
	}
}

template <class T>
void MySearch<T>::buildHashTable(T data[], int size) {
	for (int i = 0; i < size; i++) {
		hashTable[data[i].get()] = i;
	}
}

template <class T>
int MySearch<T>::cacheLookup(const string& key) {
	for (const auto& entry : cache) {
		if (entry.first == key) {
			return entry.second;
		}
	}
	return -1; // Not found in cache
}

template <class T>
void MySearch<T>::cacheInsert(const string& key, int value) {
	static size_t cacheIndex = 0;
	cache[cacheIndex] = {key, value};
	cacheIndex = (cacheIndex + 1) % cache.size();
}

template <class T>
int MySearch<T>::search(T data[], int size, char *key) {
	if (hashTable.empty()) {
		buildHashTable(data, size);
	}
	string keyStr(key);

	// Check cache first
	int cacheResult = cacheLookup(keyStr);
	if (cacheResult != -1) {
		return cacheResult;
	}

	// Check hash table
	auto it = hashTable.find(keyStr);
	if (it != hashTable.end()) {
		cacheInsert(keyStr, it->second);
		return it->second;
	}

	return -1; // Not found
}