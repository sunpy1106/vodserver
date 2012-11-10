#include "bufManagerByLFU.h"


bufManagerByLFU::bufManagerByLFU(Client*client,unsigned int blockSize,unsigned int totalBufSize):bufManager(client,blockSize,totalBufSize){
	pthread_mutex_init(&lfu_mutex,NULL);
}
bufManagerByLFU::~bufManagerByLFU(){
	pthread_mutex_destroy(&lfu_mutex);
}

int
bufManagerByLFU::initial_parameter(){
	list<LFUBlockInfo>::iterator it;
	for(it = buf.begin();it!=buf.end();it++){
		it->counts = 0;
	}
	return 0;
}

int
bufManagerByLFU::writeFileSegment(unsigned int fileId,unsigned int segId){
	pthread_mutex_lock(&lfu_mutex);
	if(buf.size()>=_blockNum){
		pthread_mutex_unlock(&lfu_mutex);
		eliminateBlock();
	}
	pthread_mutex_unlock(&lfu_mutex);
	addBlock(fileId,segId);
	return 0;
}
int
bufManagerByLFU::readFileSegment(unsigned int fileId,unsigned int segId){
	LFUBlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<LFUBlockInfo>::iterator it;
	int flag = 0;
	pthread_mutex_lock(&lfu_mutex);
	for(it = buf.begin();it!=buf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			it->counts++;
			flag = 1;
			break;
		}
	}
	pthread_mutex_unlock(&lfu_mutex);
	if(flag == 0){
		cout<<"in bufManagerByLRU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	//readBlock(fileId,segId);
	return 0;
}

int
bufManagerByLFU::haveSegment(unsigned int fileId,unsigned int segId){
	list<LFUBlockInfo>::iterator it;
	pthread_mutex_lock(&lfu_mutex);
	for(it = buf.begin();it !=buf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			pthread_mutex_unlock(&lfu_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&lfu_mutex);
	return false;
}

int
bufManagerByLFU::eliminateBlock(){
	list<LFUBlockInfo>::iterator it, minIt;
	int minWeight = 0x7fffffff;
	pthread_mutex_lock(&lfu_mutex);
	for(it = buf.begin();it !=buf.end();it++){
		if(it->counts < minWeight){
			minWeight = it->counts;
			minIt=it;
		}
	}
	buf.erase(minIt);
	pthread_mutex_unlock(&lfu_mutex);
	sendEliminateBlock(minIt->fileId,minIt->segId);
	
	return 0;
}
int
bufManagerByLFU::addBlock(unsigned int fileId,unsigned int segId){
	if(haveSegment(fileId,segId)){
		cout<<"alread have the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	LFUBlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	temp.counts = 1;
	pthread_mutex_lock(&lfu_mutex);
	buf.push_back(temp);
	pthread_mutex_unlock(&lfu_mutex);
	sendAddedBlock(fileId,segId);
	return 0;
}
