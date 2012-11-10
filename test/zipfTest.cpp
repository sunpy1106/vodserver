#include "../include/global.h"
#include "../lib/zipf.h"


int main(){
    int elements;
    double ratio;
    while(scanf("%d %lf",&elements,&ratio)!=EOF){
        zipf sample(elements,ratio);
		sample.printZipf();
//        sample.printInFile("result.txt");
        sample.printZipfInFile("result.dat");
    }
    return 0;
}
