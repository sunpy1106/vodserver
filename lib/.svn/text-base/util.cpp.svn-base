#include "util.h"
#include <iostream>
using namespace std;

const gsl_rng_type *T;
gsl_rng *r;

double Randomf(int a,int b){
	double temp = random()/(RAND_MAX*1.0);
	return a + (b-a) * temp;
}

int Randomi(int a,int b){
	double  temp = random()/(RAND_MAX * 1.0);
	return (int)(a + (b - a ) * temp) ;
}

double rand_lognormal(double zeta,double sigma){
	double result;
	result = gsl_ran_lognormal(r,zeta,sigma);
//	gsl_rng_free(r);
	return result;
}

string numToString(int n){
	char buf[256];
	sprintf(buf,"%d",n);
	string temp(buf);
	return temp;
}

int stringToInt(const string &value){
	int result;
	string value1;
	bool isNegative = false;
	if(value.size()>0 && value[0]=='-'){
		isNegative = true;
		value1 = value.substr(1);
	}
	if(isNegative == true){
		result = atoi(value1.c_str());
		result = 0 - result;
	}else
		result = atoi(value.c_str());
//	cout<<"value = "<<value<<endl;
//	cout<<"result = "<<result<<endl;
	return result;

}

int mysleep(unsigned int usec){
	struct timeval tv;
	tv.tv_sec = usec/1000000;
	tv.tv_usec = usec%1000000;
	if(select(0,NULL,NULL,NULL,&tv)!=0){
        if (errno == EINTR)
			return 0;
		else{
			perror("select sleep error");
			return -1;
		}
	}else
		return 0;
}
