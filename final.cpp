#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <string>

using namespace std;

#define MAXSTRLEN  256

char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
int strlen(const char *st);

class Str {
private:
	char st[MAXSTRLEN];
	int location;
public:
	Str();
	Str(char *s);
	void set(char *s);
	char *get();
	void setLocation(int loc) { location = loc; }
	int getLocation() { return location; }
	bool operator == (Str& s) {
		int i;
		for (i = 0; i < MAXSTRLEN && st[i] != 0 && s.st[i] != 0; i++) {
			if (tolower(st[i]) != tolower(s.st[i]))break;
		}
//		if (s.st[i] == 0) return true;
		if (s.st[i] == st[i]) return true;
		return false;
	}
	bool operator == (char *s) {
		int i;
		for (i = 0; i < MAXSTRLEN && st[i] != 0 && s[i] != 0; i++) {
			if (tolower(st[i]) != tolower(s[i]))break;
		}
		if (s[i] == st[i]) return true;
//		if (s[i] == 0) return true;
		return false;
	}
	bool operator > (Str& s) {
		int i;
		for (i = 0; i < MAXSTRLEN && st[i] != 0 && s.st[i] != 0; i++) {
			if (st[i] != s.st[i])break;
		}
		if (st[i] > s.st[i])return true;
		return false;
	}
	bool operator < (Str& s) {
		int i;
		for (i = 0; i < MAXSTRLEN && st[i] != 0 && s.st[i] != 0; i++) {
			if (st[i] != s.st[i])break;
		}
		if (st[i] < s.st[i])return true;
		return false;
	}
	friend std::ostream& operator << (std::ostream& os, Str& s);
};

std::ostream& operator << (std::ostream& os, Str& s) {
	os << (char *)s.st;
	return os;
}

template <class T>
class Sort {
protected:
	bool stop;
	char filename[4096];
	int size;
public:
	Sort() { stop = 0; }
	virtual void sort(T data[], int size)=0;
	bool load(Str s[],char *name);
	void findData(int loc,char *st,int maxlen);
	int getSize() { return size; }
	void swap(T *d1, T *d2);
	void showData(T data[], int size, int i, int j);
	void setStop(bool flag);
};

#include "student.h"

int main() {
	char tmp[4096];
	cout << "Please enter dictionary file name: ";
//	cin.getline(tmp,256);
	strcpy(tmp, "oed.txt");
	Sort<Str> *sort = new MySort<Str>();
	Str *s = new Str[40000];
	sort->load(s, tmp);
	chrono::high_resolution_clock::time_point clock1, clock2;
	clock1 = chrono::high_resolution_clock::now();
	///////////////////////////
	sort->sort(s,sort->getSize());
	///////////////////////////
	clock2 = chrono::high_resolution_clock::now();
	sort->showData(s, sort->getSize(), -1, -1);
	cout << endl << endl;
	cout << "Time passed: " << (chrono::duration_cast<chrono::duration<double>>(clock2-clock1).count())*1000 << " msec." << endl;
	cout << "After sorting " << endl;
	MySearch<Str> *search = new MySearch<Str>();
	do {
		cout << "Dictionary size = " << sort->getSize() << endl;
		cout << "Please enter a word: ";
		cin.getline(tmp, 255);
		if (tmp[0] < 0x21)break;
		clock1 = chrono::high_resolution_clock::now();
		////////////////////////////////////////////
		int loc = search->search(s,sort->getSize(), tmp);
		clock2 = chrono::high_resolution_clock::now();
		if (loc < 0)
			cout << "Definition not found!!!" << endl;
		else{
			sort->findData(s[loc].getLocation(),tmp,4096);
			cout << "===============================================" << endl;
			cout << tmp << endl;
			cout << "===============================================" << endl;
		}
		//////////////////////////////////////////////
		cout << "Time passed: " << (chrono::duration_cast<chrono::duration<double>>(clock2 - clock1).count())*1000000.0 << " usec." << endl;

	} while (true);
	cout<< "Press ENTER to continue." << endl;
	cin.get();
	return 0;
}

template <class T>
bool Sort<T>::load(Str s[],char *name) {
	char tmp[8192];
	char tmpData[MAXSTRLEN];
	int i;
	for (i = 0; name[i] != 0; i++)
		filename[i] = name[i];
	filename[i] = 0;

	size=0; // current location

	ifstream f(filename, ios::binary);
	cout << "Loading dictionary..." << (char *)name << endl;
	if (!f.is_open()) {
		cout << "Error opening file" << filename << endl;
		return false;
	}
	while (!f.eof()) {
		s[size].setLocation((int)f.tellg());
		f.getline(tmp, 8192);
		//discard empty line
		if (strlen(tmp) < 2) continue;
		for (i = 0; i < MAXSTRLEN && (tmp[i] != '.'); i++)
		    tmpData[i] = (char)tolower(tmp[i]);
		//remove word type
		i--; //move marker to the last character
        while(((tmpData[i]>0x20)||(tmpData[i]<0))&&(i>1)){
			i--;
		}
		//remove space
        while((tmpData[i]==0x20)&&(i>1)){
			i--;
		}		
		tmpData[i+1] = 0;
		//retrieve keyword
		s[size].set(tmpData);
		size++;
	}
	f.close();
	cout << "Loading complete!" << endl;
	return true;
}

template <class T>
void Sort<T>::findData(int loc,char *st,int maxlen) {
	ifstream f(filename, ios::binary);
	if (!f.is_open()) {
		cout << "Error opening file" << filename << endl;
		return;
	}
	f.seekg(loc);
	f.getline(st, maxlen);
	f.close();
}

template <class T>
void Sort<T>::swap(T *d1, T *d2) {
	T tmp = *d1;
	*d1 = *d2;
	*d2 = tmp;
}

template <class T>
void Sort<T>::showData(T data[], int size, int i, int j) {
	int x;
	if (i > -1) {
		for (x = 0; x < size; x++) {
			if (i != x) cout << "         ";
			else cout << ">> i <<<";
		}
		cout << endl;
	}
	for (x = 0; x < size; x++) {
		cout << data[x] << "][";
	}
	cout << endl;
	if (j > -1) {
		for (x = 0; x < size; x++) {
			if (j != x) cout << "         ";
			else cout << ">> j <<<";
		}
		cout << endl;
	}
}

template <class T>
void Sort<T>::setStop(bool flag) {
	stop = flag;
}

Str::Str() {
	st[0] = 0;
	location = 0;
}
Str::Str(char *s) {
	set(s);
	location = 0;
}

void Str::set(char *s) {

	int i;
	for(i=0;i<64 && s[i]!=0;i++)
		st[i] = s[i];
	st[i] = 0;
}

char *Str::get() {
	return st;
}

int strlen(const char *st) {
	int len = 0;

	while (st[len] != 0) len++;
	return len;
}

char *strcat(char *dest, const char *src) {
	strcpy(dest + strlen(dest), src);
	return dest;
}

char *strcpy(char *dest, const char *src) {
	int count;
	for (count = 0; src[count] != 0; count++)
		dest[count] = src[count];
	dest[count] = 0;
	return dest;
}