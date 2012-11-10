#include "zipf.h"

zipf::zipf(int n,double ratio){
    elements = n;
    zipfRatio = ratio;
    for(int i=0;i<elements;i++){
        result.push_back(0.0);
    }
    zipfDistribution();
}

bool
compare(const double & a,const double & b){
    return a > b ;
}

int 
zipf::zipfDistribution(){
    double base =0.0;
    double p;
    for(int j=0;j<elements;j++){
        p =  pow(j+1,zipfRatio);
        result[j] = p;
        base +=p;
    }
    for(int j=0;j<elements;j++){
        result[j] = result[j]/base;
    }
    sort(result.begin(),result.end(),compare);
    return 0;
}

int 
zipf::getZipfDistribution(vector<double> &distribution){
    distribution = result;
    return 0;
}


int 
zipf::printZipf(){
    for(int i=0;i<elements;i++){
        cout<<i+1<<"    "<<result[i]<<endl;
    }
}

int 
zipf::printZipfInFile(string pathname){
    ofstream file1(pathname.c_str());
    for(int j=0;j<elements;j++){
        file1<< j+1 <<" "<<result[j]<<endl;
    }
    file1.close();
	cout<<"write voer!"<<endl;
}
