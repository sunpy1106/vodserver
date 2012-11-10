#include <iostream>
#include "../include/global.h"
#include<map>
using namespace std;

int main(int argc,char *argv[]){
	if(argc < 3){
		cout<<"usage : ./a.out client_num resource_num"<<endl;
		return -1;
	}
	int n = atoi(argv[1]);
	int m = atoi(argv[2]);
	map<unsigned int ,unsigned int> fileCount;
	ifstream ifs;
	string configFile = "data/requestList.ini";
	int j= configFile.find('.');
	string requestListFile = (configFile).substr(0,j)+numToString(n) + (configFile).substr(j);
	ifs.open(requestListFile.c_str());
	if(ifs){
		unsigned int a,b;
		while(!ifs.eof()){
			ifs >> a>>b;
			fileCount[a]++;
		}
	}else
		return -1;
	for(int i = 0;i<m;i++){
		cout<<"there are "<<fileCount[i]<<" clients request file "<<i<<endl;
	}
	return 0;
}
