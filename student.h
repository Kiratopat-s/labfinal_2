#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <algorithm>
#include <array>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
using namespace std;

// Forward declaration of ThreadPool class
class ThreadPool;

template <class T>
class MySort:public Sort<T> {
public:
	void sort(T data[], int size);
private:
	void parallelMergeSort(T data[], int left, int right, int depth, ThreadPool& pool);
	void merge(T data[], int left, int mid, int right);
	void insertionSort(T data[], int left, int right);
	static const int INSERTION_SORT_THRESHOLD = 32; // Threshold for switching to insertion sort
	static const int MAX_THREADS = 4; // Maximum number of threads to create
};

class ThreadPool {
public:
	ThreadPool(size_t threads);
	~ThreadPool();
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args) -> future<typename result_of<F(Args...)>::type>;
private:
	vector<thread> workers;
	queue<function<void()>> tasks;
	mutex queue_mutex;
	condition_variable condition;
	bool stop;
};

ThreadPool::ThreadPool(size_t threads) : stop(false) {
	for (size_t i = 0; i < threads; ++i)
		workers.emplace_back([this] {
			for (;;) {
				function<void()> task;
				{
					unique_lock<mutex> lock(this->queue_mutex);
					this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}
				task();
			}
		});
}

ThreadPool::~ThreadPool() {
	{
		unique_lock<mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (thread &worker : workers)
		worker.join();
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> future<typename result_of<F(Args...)>::type> {
	using return_type = typename result_of<F(Args...)>::type;
	auto task = make_shared<packaged_task<return_type()>>(bind(std::forward<F>(f), std::forward<Args>(args)...));
	future<return_type> res = task->get_future();
	{
		unique_lock<mutex> lock(queue_mutex);
		if (stop)
			throw runtime_error("enqueue on stopped ThreadPool");
		tasks.emplace([task]() { (*task)(); });
	}
	condition.notify_one();
	return res;
}

template <class T>
class MySearch {
public:
	int search(T data[], int size, char *key);
	void resetHashTable();
private:
	unordered_map<string, int> hashTable;
	array<pair<string, int>, 16> cache; // Small fixed-size cache
	size_t cacheIndex = 0;
	void buildHashTable(T data[], int size);
	int cacheLookup(const string& key);
	void cacheInsert(const string& key, int value);
};

template <class T>
void MySort<T>::sort(T data[], int size) {
	ThreadPool pool(MAX_THREADS);
	int maxDepth = static_cast<int>(log2(MAX_THREADS));
	parallelMergeSort(data, 0, size - 1, maxDepth, pool);
}

template <class T>
void MySort<T>::parallelMergeSort(T data[], int left, int right, int depth, ThreadPool& pool) {
	if (left >= right) return;

	if (right - left + 1 <= INSERTION_SORT_THRESHOLD) {
		insertionSort(data, left, right);
		return;
	}

	if (depth <= 0) {
		std::sort(data + left, data + right + 1);
	} else {
		int mid = left + (right - left) / 2;
		auto leftResult = pool.enqueue(&MySort::parallelMergeSort, this, data, left, mid, depth - 1, std::ref(pool));
		auto rightResult = pool.enqueue(&MySort::parallelMergeSort, this, data, mid + 1, right, depth - 1, std::ref(pool));
		leftResult.get();
		rightResult.get();
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
	hashTable.clear();
	for (int i = 0; i < size; i++) {
		hashTable[data[i].get()] = i;
	}
}

template <class T>
void MySearch<T>::resetHashTable() {
	hashTable.clear();
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
