#include "bufManagerByLFRU.h"
#include <pthread.h>

void LFRUBlockResetThread(void * arg){
	bufManagerByLFRU *bufmanager = (bufManagerByLFRU*)arg;
	bufmanager->blockReset();
}

bufManagerByLFRU::bufManagerByLFRU(Client*client,unsigned int blockSize,unsigned int totalBufSize,unsigned int period):bufManager(client,blockSize,totalBufSize){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	t0 = tv.tv_sec * 1000000 + tv.tv_usec;
	pthread_mutex_init(&buf_mutex,NULL);
	if(period ==0)
		period = 500;
	_period = period;
}

bufManagerByLFRU::~bufManagerByLFRU(){
	pthread_mutex_destroy(&buf_mutex);

}

void 
bufManagerByLFRU::blockReset( ){//D这里到时候要在配置文件中设的，我暂时在头文件里define了。
	struct timeval tv;
	gettimeofday(&tv,NULL);
	list<LFRUBlockInfoo>::iterator it;
	pthread_mutex_lock(&buf_mutex);
	t0 = tv.tv_sec * 1000000 + tv.tv_usec;
	for(it = buf.begin();it!=buf.end();it++){
		it->weight = 0.0;
		it->periodCounter = 1;
		it->lastAccessTime = t0;
	}
	timeOver();
	pthread_mutex_unlock(&buf_mutex);
}

int
bufManagerByLFRU::writeFileSegment(unsigned int fileId,unsigned int segId){//要加锁，要防止中间被访问而该参数。
	if(buf.size()==0){
		myAlarmEvent.period = _period;
		myAlarmEvent.timeleft = _period;
		myAlarmEvent.callBackFunc = &LFRUBlockResetThread;
		myAlarmEvent.arg = this;
		addAlarmEvent(&myAlarmEvent);
		cout<<"add the alarm event ..."<<endl;
	}
	pthread_mutex_lock(&buf_mutex);
	if(buf.size() >=_blockNum){
		pthread_mutex_unlock(&buf_mutex);
		eliminateBlock();
	}else
		pthread_mutex_unlock(&buf_mutex);
	addBlock(fileId,segId);
	return 0;
}
int
bufManagerByLFRU::readFileSegment(unsigned int fileId,unsigned int segId){
	LFRUBlockInfoo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<LFRUBlockInfoo>::iterator it;
	int flag = 0;
	struct timeval tv;
	pthread_mutex_lock(&buf_mutex);
	for(it = buf.begin();it!=buf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			it->periodCounter++;
			gettimeofday(&tv,NULL);
			it->lastAccessTime = tv.tv_sec * 1000000 + tv.tv_usec;
			flag = 1;
			break;
		}
	}
	pthread_mutex_unlock(&buf_mutex);
	if(flag == 0){
		cout<<"in bufManagerByLRU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	//readBlock(fileId,segId);
	return 0;
}

int
bufManagerByLFRU::haveSegment(unsigned int fileId,unsigned int segId){
	list<LFRUBlockInfoo>::iterator it;
	pthread_mutex_lock(&buf_mutex);
	for(it = buf.begin();it !=buf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			pthread_mutex_unlock(&buf_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&buf_mutex);
	return false;
}

int
bufManagerByLFRU::eliminateBlock(){
	double Fk;
	unsigned long  Rk;
	unsigned int fileId,segId;
	list<LFRUBlockInfoo>::iterator it, maxIt;
	double maxWeight =0.0;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	pthread_mutex_lock(&buf_mutex);
	recallTime = tv.tv_sec * 1000000 + tv.tv_usec;
	for(it = buf.begin();it !=buf.end();it++){
		if(it->periodCounter ==0){
			cout<<"counter error"<<endl;
			exit(1);
		}
		Fk = (recallTime - t0)/(double)(it->periodCounter);
		Rk = recallTime - (it->lastAccessTime);
		it->weight = ((unsigned long)_period- (recallTime-t0)) / (double)_period * Rk + (recallTime - t0)/ (double)_period * Fk;
		if(it->weight > maxWeight){
			maxWeight = it->weight;
			maxIt=it;
		}
	}
	fileId = maxIt->fileId;
	segId = maxIt->segId;
	buf.erase(maxIt);
	pthread_mutex_unlock(&buf_mutex);
	sendEliminateBlock(fileId,segId);
	return 0;
}
int
bufManagerByLFRU::addBlock(unsigned int fileId,unsigned int segId){
	if(haveSegment(fileId,segId)){
		cout<<"alread have the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	LFRUBlockInfoo temp;
	temp.fileId = fileId;
	temp.segId = segId;
	temp.weight = 0.0;
	temp.periodCounter = 1;
	pthread_mutex_lock(&buf_mutex);
	temp.lastAccessTime = t0;
	buf.push_back(temp);
	pthread_mutex_unlock(&buf_mutex);
	sendAddedBlock(fileId,segId);
	return 0;
}
