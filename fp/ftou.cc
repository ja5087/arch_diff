#include <iostream>
#include <cmath>

using namespace std;

int main() {
    // Convert a double to a unsigned long long for stable copying
    double a = 0;
    while (true) {
        cin>>a;
        unsigned long long res = *(unsigned long long*)(&a);
        cout << "value in ul " << res << endl;
    }    
	return 0;
}
