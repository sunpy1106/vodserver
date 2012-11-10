#include "../include/global.h"

#include <iostream>
using namespace std;
extern const gsl_rng_type *T;
extern gsl_rng *r;
int main(){
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	double zeta ;
	double sigma ;
	while(true){
		cout<<"please input the zeta :"<<endl;
		cin >> zeta;
		cout<<"please input the sigma :"<<endl;
		cin >>sigma;
		cout<<"zeta = "<<zeta<<"\tsigma = "<<sigma<<endl;
		for(int i = 1;i<= 100;i++){
			cout<<rand_lognormal(zeta,sigma)<<"\t";
			if(i% 7==0)
				cout<<endl;
		}
		cout<<endl;
	}
	return 0;
}
