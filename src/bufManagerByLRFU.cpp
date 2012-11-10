#include "bufManagerByLRFU.h"
#include <float.h>


bufManagerByLRFU::bufManagerByLRFU(Client* client,unsigned int blockSize,unsigned int totalBufSize,float lambda)
	:bufManager(client,blockSize,totalBufSize){
	_lambda = lambda;
}

bufManagerByLRFU::~bufManagerByLRFU(){
}

int
bufManagerByLRFU::initial_parameter(){
	list<lrfuBlockInfo>::iterator it;
	for(it = lrfuBuf.begin();it!=lrfuBuf.end();it++){
		it->weight = 0.0;
	}
	return 0;
}

int
bufManagerByLRFU::writeFileSegment(unsigned int fileId,unsigned int segId){
	if(lrfuBuf.size()>=_blockNum){
		eliminateBlock();
	}
	addBlock(fileId,segId);
	return 0;
}
int
bufManagerByLRFU::readFileSegment(unsigned int fileId,unsigned int segId){
	lrfuBlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<lrfuBlockInfo>::iterator it;
	int accessTime;
	int flag = 0;
	for(it = lrfuBuf.begin();it!=lrfuBuf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			gettimeofday(&tv,NULL);
			accessTime = tv.tv_sec * 1000000 + tv.tv_usec;
			it->weight = (it->lastWeight)*pow(0.5,((accessTime - (it->lastTime))*_lambda)/1000000.0) + 1.0;
			it->lastWeight = it->weight;
			it->lastTime = accessTime;
			flag = 1;
			break;
		}
	}
	if(flag == 0){
		cout<<"in bufManagerByLRFU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	return 0;
}

int
bufManagerByLRFU::haveSegment(unsigned int fileId,unsigned int segId){
	list<lrfuBlockInfo>::iterator it;
	for(it = lrfuBuf.begin();it !=lrfuBuf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			return true;
		}
	}
	return false;
}

int
bufManagerByLRFU::eliminateBlock(){
	list<lrfuBlockInfo>::iterator it, minIt;
	float minWeight = FLT_MAX;
	int baseTime;
	gettimeofday(&tv,NULL);
	baseTime = tv.tv_sec * 1000000 + tv.tv_usec;
	for(it = lrfuBuf.begin();it !=lrfuBuf.end();it++){
		it->weight = (it->lastWeight)*pow(0.5,((baseTime - (it->lastTime))*_lambda)/1000000.0) + 1.0;
		if(it->weight < minWeight){
			minWeight = it->weight;
			minIt=it;
		}
	}
	sendEliminateBlock(minIt->fileId,minIt->segId);
	lrfuBuf.erase(minIt);
	return 0;
}
int
bufManagerByLRFU::addBlock(unsigned int fileId,unsigned int segId){
	if(haveSegment(fileId,segId)){
		cout<<"alread have the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	lrfuBlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	gettimeofday(&tv,NULL);
	temp.lastTime = tv.tv_sec * 1000000 + tv.tv_usec;
	temp.lastWeight = 1.0;
	lrfuBuf.push_back(temp);
	sendAddedBlock(fileId,segId);
	return 0;
}
