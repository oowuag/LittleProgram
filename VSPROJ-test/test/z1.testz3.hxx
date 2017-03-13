#include <iostream>       // std::cout

#include <stack>
#include <string>

#include <vector>
#include <cstdlib>

using namespace std;

int main2()
{
  std::stack<int> mystack;

  for (int i=0; i<5; ++i) mystack.push(i);

  std::cout << "Popping out elements...";
  while (!mystack.empty())
  {
     std::cout << ' ' << mystack.top();
     mystack.pop();
  }
  std::cout << '\n';

  return 0;
}


int solution(string &S) {

    stack<int> stk;

    for(int i = 0; i < S.size(); i++) {

        int x = S.at(i);
        if (x >= '0' && x <= '9') {
            x -= '0';
        }
        else if (x == '+') {
            x = -1;
        }
        else if (x == '*') {
            x = -2;
        }
        stk.push(x);

        while (!stk.empty() && (stk.top() == -1 || stk.top() == -2)) {
            int x0 = stk.top();
            stk.pop();
            if (stk.empty()) {
                break;
            }
            int x1 = stk.top();
            stk.pop();
            if (stk.empty()) {
                break;
            }
            int x2 = stk.top();
            stk.pop();

            int x3 = 0;
            if (-1 == x0) {
                x3 = x1 + x2;
            }
            else if (-2 == x0) {
                x3 = x1 * x2;
            }
            else {
            }
            stk.push(x3);
        }
    }

    if (stk.size() == 1 && stk.top() != -1 && stk.top() != -2) {
        return stk.top();
    }
    else {
	    return -1;
    }
}

//int solution2(string &S) {
//	vector<int> vec;
//	int oper1, oper2;
//	for (string::iterator it=S.begin(); it!=S.end(); ++it) {
//		if (*it == '+' || *it == '*') {
//			if (--it != S.begin()) {
//				oper1 = *it - '\0';
//				*it = ' ';
//				if (--it != S.begin()) {
//					oper2 = *it - '\0';
//					if (*it == '+') {
//						*it = oper1 + oper2;
//					}
//					else if (*it == '*') {
//						*it = oper1 * oper2;
//					}
//					
//					if (*it > 0xFFF) {
//						return -1;
//					}
//			}
//		}
//		else if (*it == ' ') {
//			//while (--it != ' ');
//		}
//		else {
//			vec.insert(it,atoi(*it));
//		}
//    }
//}

int main()
{
    //string s = "13+62*7+*";
    string s = "13+*";


    cout << solution(s) << endl;
    //cout << solution2(s) << endl;
    
    return 0;
}
   