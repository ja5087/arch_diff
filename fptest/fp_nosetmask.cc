#include <iostream>

using namespace std;

int main() {
        unsigned int a,b,c,d;
        cin>>a>>b>>c>>d;
        float* af = (float*) &a;
        float* bf = (float*) &b;
        float* cf = (float*) &c;
        float* df = (float*) &d;
	
        // cross product
        float cp = (*af)*(*df) - (*bf)*(*cf);
        unsigned int res = *(unsigned int*)(&cp);
        cout << res << endl;
	return 0;
}
