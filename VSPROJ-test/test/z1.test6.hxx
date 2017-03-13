#include <iostream>
using namespace std;

//// you can use includes, for example:
//// #include <algorithm>
//
//// you can write to stdout for debugging purposes, e.g.
//// cout << "this is a debug message" << endl;
//
//#include <iostream>
//#include <vector>
//using namespace std;
//
//
//int solution(vector<int> &A) {
//    // write your code in C++11 (g++ 4.8.2)
//    cout << "this is a debug message" << endl;
//
//
//    return 0;
//}


/////////////
#include <vector>
#include <algorithm>

int solution(vector<int> &A, vector<int> &B) {
    int n = A.size();
    int m = B.size();
    sort(A.begin(), A.end());
    sort(B.begin(), B.end());
    int i = 0;
    for (int k = 0; k < n; k++) {
        while (i < m - 1 && B[i] < A[k])
            i++;
        if (A[k] == B[i])
            return A[k];
    }
    return -1;

}

int main()
{
    vector<int> A;
    vector<int> B;

    A.push_back(11);
    A.push_back(12);
    A.push_back(19);

    B.push_back(32);
    B.push_back(35);
    B.push_back(37);


    cout << solution(A, B) << endl;
    
}
