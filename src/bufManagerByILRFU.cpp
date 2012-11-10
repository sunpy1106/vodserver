#include "bufManagerByILRFU.h"
#include <float.h>

bufManagerByILRFU::bufManagerByILRFU(){
	MODIFY_TIMES=5;
	MODIFY_RATIO=1.0;
	MODIFY_VALUE=5.0;							
	inTimes = 1;
	outTimes = 1;								
	succTimes = 0;									
	isInQout = 0;	
}

bufManagerByILRFU::~bufManagerByILRFU(){
}

int
bufManagerByILRFU::initial_parameter(){
	lambda = 0.99;
	list<BlockInfo>::iterator it;
	for(it = buf.begin();it!=buf.end();it++){
		it->weight = 0.0;
	}
	return 0;
}

int
bufManagerByILRFU::writeFileSegment(unsigned int fileId,unsigned int segId){
	if(buf.size()>=_blockNum){
		eliminateBlock();
	}
	addBlock(fileId,segId);
	return 0;
}
int
bufManagerByILRFU::readFileSegment(unsigned int fileId,unsigned int segId){
	BlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<BlockInfo>::iterator it;
	int accessTime;
	int flag = 0;
	for(it = buf.begin();it!=buf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			accessTime = tv.tv_sec * 1000 + tv.tv_usec;
			it->weight = (it->lastWeight)*pow(0.5,((accessTime - (it->lastTime))*lambda)) + 1.0;
			it->lastWeight = it->weight;
			it->lastTime = accessTime;
			flag = 1;
			inTimes++;
			isInQout = 1;
			break;
		}
	}
	if(isPreIn==isInQout){
		succTimes++;
	}
	else{
		succTimes=0;
	}
	isPreIn=isInQout;

	if(((inTimes >MODIFY_TIMES) && (inTimes/outTimes >MODIFY_RATIO))||((succTimes>MODIFY_TIMES) && (isPreIn==1))){
		lambda[rNum]/=MODIFY_VALUE;
		inTimes[rNum]=1;
		outTimes[rNum]=1;
		succTimes[rNum]=0;
	}

	if(((outTimes>MODIFY_TIMES) && (outTimes/inTimes >MODIFY_RATIO))||((succTimes>MODIFY_TIMES) && (isPreIn==0))){
		lambda[rNum]*=MODIFY_VALUE;
		if(lambda[rNum]>=1){
			lambda[rNum]=0.9999;
		}
		inTimes[rNum]=1;
		outTimes[rNum]=1;
		succTimes[rNum]=0;

	}
	
	if(flag == 0){
		cout<<"in bufManagerByILRFU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	return 0;
}

int
bufManagerByILRFU::haveSegment(unsigned int fileId,unsigned int segId){
	list<BlockInfo>::iterator it;
	for(it = buf.begin();it !=buf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			return true;
		}
	}
	return false;
}

int
bufManagerByILRFU::eliminateBlock(){
	list<BlockInfo>::iterator it, minIt;
	float minWeight = FLT_MAX;
	int baseTime;
	baseTime = tv.tv_sec * 1000 + tv.tv_usec;
	for(it = buf.begin();it !=buf.end();it++){
		it->weight = (it->lastWeight)*pow(0.5,((baseTime - (it->lastTime))*lambda)) + 1.0;
		if(it->weight < minWeight){
			minWeight = it->weight;
			minIt=it;
		}
	}
	QOut qTemp;
	if(qBuf.size()>=_qOutStore){
		qBuf.pop_front();
	}
	qTemp.fileId = minIt->fileId;
	qTemp.segId = minIt->segId;
	qTemp.weight = minIt->weight;
	qBuf.push_back(qTemp);
	}
	sendEliminateBlock(minIt->fileId,minIt->segId);
	buf.erase(minIt);
	}
	return 0;
}

int
bufManagerByILRFU::addBlock(unsigned int fileId,unsigned int segId){
	int flag = 0;
	if(haveSegment(fileId,segId)){
		cout<<"alread have the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	BlockInfo temp;
	list<Qout>::iterator qoutIt;
	for(qoutIt = qBuf.begin();qoutIt !=qBuf.end();qoutIt++){
		if(qoutIt->fileId == fileId && qoutIt->segId == segId){
			flag = 1;
			temp.fileId = fileId;
			temp.segId = segId;
			temp.lastTime = tv.tv_sec * 1000 + tv.tv_usec;
			temp.lastWeight = qoutIt->weight;
			buf.push_back(temp);
			qBuf.erase(qoutIt);
			
			inTimes++;
			isInQout = 1;
			
			break;
		}
	}
	if(flag = 0){
		temp.fileId = fileId;
		temp.segId = segId;
		temp.lastTime = tv.tv_sec * 1000 + tv.tv_usec;
		temp.lastWeight = 1.0;
		buf.push_back(temp);
		outTimes++;
		sendAddedBlock(fileId,segId);
	}
	
	if(isPreIn==isInQout){						//连续在抛弃队列中的话。。。
		succTimes++;
	}
	else{
		succTimes= 0;
	}
	isPreIn=isInQout;
	
	if(((inTimes >MODIFY_TIMES) && (inTimes/outTimes >MODIFY_RATIO))||((succTimes>MODIFY_TIMES) && (isPreIn==1))){
		lambda[rNum]/=MODIFY_VALUE;
		inTimes[rNum]=1;
		outTimes[rNum]=1;
		succTimes[rNum]=0;
	}

	if(((outTimes>MODIFY_TIMES) && (outTimes/inTimes >MODIFY_RATIO))||((succTimes>MODIFY_TIMES) && (isPreIn==0))){
		lambda[rNum]*=MODIFY_VALUE;
		if(lambda[rNum]>=1){
			lambda[rNum]=0.9999;
		}
		inTimes[rNum]=1;
		outTimes[rNum]=1;
		succTimes[rNum]=0;

	}
	return 0;
}
