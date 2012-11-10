#include "bufManagerByPLFU.h"
#include <float.h>


bufManagerByPLFU::~bufManagerByPLFU(){
}

int
bufManagerByPLFU::initial_parameter(){
	list<BlockInfo>::iterator it;
	for(it = buf.begin();it!=buf.end();it++){
		it->weight = 0.0;
		it->Ck = 0;
		it->Ck_1 = 0;
		it->Ck_2 = 0;
		it->Ck_3 = 0;
		it->periodCounter = 0;
	}
	return 0;
}

int
bufManagerByPLFU::sig_alarm(int signo){//T这里到时候要在配置文件中设的，我暂时在头文件里define了。
	alarm(T);
	t0 = tv.tv_sec * 1000 + tv.tv_usec;
	list<BlockInfo>::iterator it;
	for(it = buf.begin();it!=buf.end();it++){
		it->Ck_3 = it->Ck_2;
		it->Ck_2 = it->Ck_1;
		it->Ck_1 = it->Ck;
		it->periodCounter = 0;
	}
	return 0;
}

int
bufManagerByPLFU::writeFileSegment(unsigned int fileId,unsigned int segId){
	if(buf.size()>=_blockNum){
		eliminateBlock();
	}
	addBlock(fileId,segId);
	return 0;
}
int
bufManagerByPLFU::readFileSegment(unsigned int fileId,unsigned int segId){
	BlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<BlockInfo>::iterator it;
	int flag = 0;
	for(it = buf.begin();it!=buf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			it->Ck++;
			it->periodCounter++;
			flag = 1;
			break;
		}
	}
	if(flag = 0){
		cout<<"in bufManagerByPLFU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	return 0;
}

int
bufManagerByPLFU::haveSegment(unsigned int fileId,unsigned int segId){
	list<BlockInfo>::iterator it;
	for(it = buf.begin();it !=buf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			return true;
		}
	}
	return false;
}

int
bufManagerByPLFU::eliminateBlock(){
	list<BlockInfo>::iterator it, minIt;
	int forecast_N;
	int currentTime;
	currentTime = tv.tv_sec * 1000 + tv.tv_usec - t0;
	int minWeight = FLT_MAX;
	for(it = buf.begin();it !=buf.end();it++){
		forecast_N = (it->Ck_1 - it->Ck_2) + ((it->Ck_1 - it->Ck_2) - (it->Ck_2 - it->Ck_3))
		it->weight = (T - currentTime)/T * forecast_N + (it->periodCounter);
		if(it->weight < minWeight){
			minWeight = it->weight;
			minIt=it;
		}
	}
	sendEliminateBlock(minIt->fileId,minIt->segId);
	buf.erase(minIt);
	}
	return 0;
}
int
bufManagerByPLFU::addBlock(unsigned int fileId,unsigned int segId){
	if(haveSegment(fileId,segId)){
		cout<<"alread have the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	BlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	it->Ck = 1;
	it->Ck_1 = 0;
	it->Ck_2 = 0;
	it->Ck_3 = 0;
	it->periodCounter = 1;
	buf.push_back(temp);
	sendAddedBlock(fileId,segId);
	return 0;
}
