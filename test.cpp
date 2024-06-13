// test.cpp
#include <iostream>
#include "MemoryLoan.h"
using namespace std;

int main() {
	char chArray[4]{'0','1','2','3'};
	MemoryLoan mp(chArray, sizeof(chArray));
	char *pch = mp.Lend();
	cout << *pch;
	mp.Return(pch);
	pch = mp.Lend();
	cout << *pch << endl;
	mp.Return(pch);
}
