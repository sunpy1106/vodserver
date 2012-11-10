/****************************************************
	产生N=100个在（0,50）区间内满足对数正态分布的随机数
/*****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define N 100 
#define MAX 50
#define MIN 0.1
#define MIU 0.01
#define SIGMA 8
#define PI 3.14159

double AverageRandom(double min,double max)//产生(min,max)之间均匀分布的随机数
{
        int MINnteger = (int)(min*10000);
        int MAXnteger = (int)(max*10000);
        int randInteger = rand()*rand();
        int diffInteger = MAXnteger - MINnteger;
        int resultInteger = randInteger % diffInteger + MINnteger;
        return resultInteger/10000.0;
}
double LogNormal(double x,double miu,double sigma) //对数正态分布概率密度函数
{
        return 1.0/(x*sqrt(2*PI)*sigma * exp((log(x)-miu)*(log(x)-miu)/(2*sigma*sigma)));
}
double Random_LogNormal(double miu,double sigma,double min,double max)//产生对数正态分布随机数
{
        double x;
        double dScope;
        double y;
        do
        {
			x = AverageRandom(min,max); 
			y = LogNormal(x, miu, sigma);
			dScope = AverageRandom(0, LogNormal(miu,miu,sigma));
        }while( dScope > y);
        return x;
}
int 
main(void)
{
	srand(time(NULL));
	int i,j;
	for(i=0,j=0;i<N;i++)
	{
		printf("%14f", Random_LogNormal(MIU,SIGMA,MIN,MAX));
		j++;
		if(j==5)
		{
			printf("\n");	//每行显示5个数
			j=0;
		}

	}
	return 0;
}
