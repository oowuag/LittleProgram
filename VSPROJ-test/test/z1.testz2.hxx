#include <iostream>
using namespace std;

//// you can use includes, for example:
//// #include <algorithm>
//
//// you can write to stdout for debugging purposes, e.g.
//// cout << "this is a debug message" << endl;
//
//#include <iostream>
#include <vector>
//using namespace std;
//
//
//int solution(int A, int B) {
//
//    vector<int> vA;
//    vector<int> vB;
//
//    do {
//        vA.push_back(A % 10); 
//    }while(A / 10 != 0);
//
//    do {
//        vA.push_back(B % 10); 
//    }while(B / 10 != 0);
//
//
//    vector<int> vC;
//    for (int i = 0; i < vA.size() && i < vB.size(); i++) {
//    }
//
//        do {
//        vA.push_back(B % 10); 
//    }while(B / 10 != 0);
//
//    return 0;
//}


#include <cstdlib>

int solution(int A, int B) {
    char strA[10] = { 0 };
    char strB[10] = { 0 };
    char strAB[20] = { 0 };

    sprintf(strA,"%d",A);
    sprintf(strB,"%d",B);
	
	int i = 0;
	while(strA[i] != '\0' && strB[i] != '\0') {
		strAB[2*i] = strA[i];
		strAB[2*i+1] = strB[i];
		i++;
	}
	
	int j = 2*i;
	
	while (strA[i] != '\0') {
		strAB[j++] = strA[i++];
	}
	
	while (strB[i] != '\0') {
		strAB[j++] = strB[i++];
	}
	
	strAB[j] = '\0';
    if (j > 10) {
        return -1;
    }
    int C = atoi(strAB);

    if (C > 100000000) {
        return -1;
    }
	
	return C;
}



int main()
{
    int A;
    int B;

    A = 100000001;
    B = 0;


    cout << solution(A, B) << endl;
    
}
