#ifndef UTIL_H
#define UTIL_H
#include<sys/select.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include<string>
#include<sys/types.h>
#include<unistd.h>
using namespace std;


typedef struct timeCallBack{
	unsigned int period;
	unsigned int timeleft;
	unsigned int clockInfo;
	void (*callBackFunc)(void *arg);
	void *arg;
}TimeCallBack;

double Randomf(int a,int b);
int Randomi(int a,int b);
double rand_lognormal(double zeta,double sigma);
string numToString(int num);
int stringToInt(const string &value);
int mysleep(unsigned int usec);

#endif
