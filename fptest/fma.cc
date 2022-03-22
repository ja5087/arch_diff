#include <iostream>
#include <cmath>

using namespace std;

int main() {
    unsigned long long a,b,c;
    cin>>a>>b>>c;
    double* af = (double*) &a;
    double* bf = (double*) &b;
    double* cf = (double*) &c;
	
	double fma_res = std::fma(*af, *bf, *cf);
    double reg_res = (*af)*(*bf)+(*cf);

    unsigned long long fma_res_int = *(unsigned long long*)(&fma_res);
    unsigned long long reg_res_int = *(unsigned long long*)(&reg_res);

    cout << "FMA Result: " << fma_res_int << endl;
    cout << "a*b + c Result: " << reg_res_int << endl;
	return 0;
}
