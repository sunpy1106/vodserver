#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

vector<unsigned int > buf;

int printBuf(){
	vector<unsigned int>::iterator it;
	for(it = buf.begin();it!=buf.end();it ++){
//		cout<<*it<<" ";
	}
//	cout<<endl;
	return 0;
}

int showBuf(unsigned int fileId,unsigned int segId,const string &action,const string& algo){
	vector<unsigned int>::iterator it;
	if(action.compare("Reset")== 0){
		cout<<"Reset\t"<<printBuf();
		return 0;
	}
	if(algo.compare("DW")==0){
		if(action.compare("ADD")==0){
			buf.push_back(segId);
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}else if(action.compare("Read") == 0){
			for(it = buf.begin();it!=buf.end();it++){
				if(*it == segId){
					buf.erase(it);
					break;
				}
			}
			buf.push_back(segId);
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}else if(action.compare("Eliminate")==0){
			for(it = buf.begin();it!=buf.end();it++){
				if(*it == segId){
					buf.erase(it);
					break;
				}
			}
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}
	}else if(algo.compare("LRU") == 0){
		if(action.compare("ADD")==0){
			buf.push_back(segId);
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;

		}else if(action.compare("Read") == 0){
			for(it = buf.begin();it!=buf.end();it++){
				if(*it == segId){
					buf.erase(it);
					break;
				}
			}
			buf.push_back(segId);
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}else if(action.compare("Eliminate")==0){
			for(it = buf.begin();it!=buf.end();it++){
				if(*it == segId){
					buf.erase(it);
					break;
				}
			}
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}
	}else if(algo.compare("LFU") == 0){
		if(action.compare("ADD")==0){
			buf.push_back(segId);
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}else if(action.compare("Read") == 0){
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}else if(action.compare("Eliminate")==0){
			for(it = buf.begin();it!=buf.end();it++){
				if(*it == segId){
					buf.erase(it);
					break;
				}
			}
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}
	}else if(algo.compare("LFRU") == 0){
		if(action.compare("ADD")==0){
			buf.push_back(segId);
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}else if(action.compare("Read") == 0){
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}else if(action.compare("Eliminate")==0){
			for(it = buf.begin();it!=buf.end();it++){
				if(*it == segId){
					buf.erase(it);
					break;
				}
			}
			cout<<action<<" "<<segId<<"\t"<<printBuf()<<endl;
		}
	}
	return 0;
}

int 
main(int argc,char* argv[]){
	if(argc < 3){
		cout<<"usage: ./showBuf fileName algorithm"<<endl;
		return -1;
	}
	ifstream bufIfs;
	bufIfs.open(argv[1]);
	string algorithm(argv[2]);
	unsigned int fileId,segId;
	string action;
	buf.clear();
	while(!bufIfs.eof()){
		bufIfs >> fileId >> segId >> action;
		showBuf(fileId,segId,action,algorithm);
	}
	return 0;
}
