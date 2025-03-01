#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <future>
#include <unordered_map>
using namespace std;

template <class T>
class MySort:public Sort<T> {
public:
	void sort(T data[], int size);
private:
	void parallelMergeSort(T data[], int left, int right, int depth);
	void merge(T data[], int left, int mid, int right);
};

class TrieNode {
public:
	unordered_map<char, TrieNode*> children;
	bool isEndOfWord;
	int location;
	TrieNode() : isEndOfWord(false), location(-1) {}
};

class Trie {
private:
	TrieNode* root;
	void insert(TrieNode* node, const string& key, int location, size_t index);
	TrieNode* search(TrieNode* node, const string& key, size_t index);
public:
	Trie();
	void insert(const string& key, int location);
	int search(const string& key);
};

Trie::Trie() {
	root = new TrieNode();
}

void Trie::insert(const string& key, int location) {
	insert(root, key, location, 0);
}

void Trie::insert(TrieNode* node, const string& key, int location, size_t index) {
	if (index == key.size()) {
		node->isEndOfWord = true;
		node->location = location;
		return;
	}
	char ch = tolower(key[index]); // Ensure case-insensitive insertion
	if (node->children.find(ch) == node->children.end()) {
		node->children[ch] = new TrieNode();
	}
	insert(node->children[ch], key, location, index + 1);
}

TrieNode* Trie::search(TrieNode* node, const string& key, size_t index) {
	if (index == key.size()) {
		return node;
	}
	char ch = tolower(key[index]); // Ensure case-insensitive search
	if (node->children.find(ch) == node->children.end()) {
		return nullptr;
	}
	return search(node->children[ch], key, index + 1);
}

int Trie::search(const string& key) {
	TrieNode* node = search(root, key, 0);
	if (node != nullptr && node->isEndOfWord) {
		return node->location;
	}
	return -1;
}

template <class T>
class MySearch {
private:
	Trie trie;
	void buildTrie(T data[], int size);
	int parallelTrieSearch(const string& key, int depth);
public:
	int search(T data[], int size, char *key);
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
void MySearch<T>::buildTrie(T data[], int size) {
	for (int i = 0; i < size; i++) {
		trie.insert(data[i].get(), i);
	}
}

template <class T>
int MySearch<T>::parallelTrieSearch(const string& key, int depth) {
	if (depth <= 0) {
		return trie.search(key);
	} else {
		std::future<int> result = std::async([this, key]() { return trie.search(key); });
		return result.get();
	}
}

template <class T>
int MySearch<T>::search(T data[], int size, char *key) {
	buildTrie(data, size);
	string keyStr(key);
	int maxDepth = std::thread::hardware_concurrency();
	return parallelTrieSearch(keyStr, maxDepth);
}
