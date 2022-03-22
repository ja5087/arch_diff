#include <iostream>
#include <fpu_control.h>

using namespace std;

int main() {
        unsigned int a,b,c,d;
        cin>>a>>b>>c>>d;
        float* af = (float*) &a;
        float* bf = (float*) &b;
        float* cf = (float*) &c;
        float* df = (float*) &d;
	
	fpu_control_t mask;
	_FPU_GETCW(mask);
	mask &= ~_FPU_EXTENDED; // disable 80-bit ext prec
	mask |= _FPU_SINGLE; // enable single prec
	_FPU_SETCW(mask);
        // cross product
        float cp = (*af)*(*df) - (*bf)*(*cf);
        unsigned int res = *(unsigned int*)(&cp);
        cout << res << endl;
	return 0;
}
