#include "bufManagerByLRU.h"


bufManagerByLRU::bufManagerByLRU(Client*client,unsigned int blockSize,unsigned int totalBufSize):bufManager(client,blockSize,totalBufSize){
	pthread_mutex_init(&lru_mutex,NULL);
}
bufManagerByLRU::~bufManagerByLRU(){
	pthread_mutex_destroy(&lru_mutex);
}

int
bufManagerByLRU::writeFileSegment(unsigned int fileId,unsigned int segId){
	pthread_mutex_lock(&lru_mutex);
	if(buf.size()>=_blockNum){
		pthread_mutex_unlock(&lru_mutex);
		eliminateBlock();
	}
	pthread_mutex_unlock(&lru_mutex);
	addBlock(fileId,segId);
	return 0;
}
int
bufManagerByLRU::readFileSegment(unsigned int fileId,unsigned int segId){
	BlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<BlockInfo>::iterator it;
	pthread_mutex_lock(&lru_mutex);
	for(it = buf.begin();it!=buf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			break;
		}
	}
	if(it!=buf.end()){
		buf.erase(it);
		buf.push_back(temp);
		pthread_mutex_unlock(&lru_mutex);
	}else{
		pthread_mutex_unlock(&lru_mutex);
		cout<<"in bufManagerByLRU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	//readBlock(fileId,segId);
	return 0;
}

int
bufManagerByLRU::haveSegment(unsigned int fileId,unsigned int segId){
	list<BlockInfo>::iterator it;
	pthread_mutex_lock(&lru_mutex);
	for(it = buf.begin();it !=buf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			pthread_mutex_unlock(&lru_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&lru_mutex);
	return false;
}
int
bufManagerByLRU::eliminateBlock(){
	BlockInfo del;;
	pthread_mutex_lock(&lru_mutex);
	del = buf.front();
	buf.pop_front();
	pthread_mutex_unlock(&lru_mutex);
	sendEliminateBlock(del.fileId,del.segId);
	return 0;
}
int
bufManagerByLRU::addBlock(unsigned int fileId,unsigned int segId){
	if(haveSegment(fileId,segId)){
		cout<<"alread have the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	BlockInfo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	pthread_mutex_lock(&lru_mutex);
	buf.push_back(temp);
	pthread_mutex_unlock(&lru_mutex);
	sendAddedBlock(fileId,segId);
	return 0;
}
