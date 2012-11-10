#include "bufManagerByDW.h"

void DWBlockResetThread(void *arg){
	bufManagerByDW *bufmanager = (bufManagerByDW*)arg;
	bufmanager->blockReset();
}

bufManagerByDW::bufManagerByDW(Client*client,unsigned int blockSize,unsigned int totalBufSize,unsigned int period):bufManager(client,blockSize,totalBufSize){
	if(period ==0)
		period = 500;
	_period = period;
	pthread_mutex_init(&dwbuf_mutex,NULL);
}


bufManagerByDW::~bufManagerByDW(){
	pthread_mutex_destroy(&dwbuf_mutex);
}


void
bufManagerByDW::blockReset(){//T这里到时候要在配置文件中设的，我暂时在头文件里define了。
	list<DWBlockInfo*>::iterator it;
	pthread_mutex_lock(&dwbuf_mutex);
	for(it = lruQueue.begin();it!=lruQueue.end();it++){
		(*it)->m_histOld = (*it)->m_histNew;
		//(*it)->m_histOld = (*it)->m_histOld   + (*it)->m_histNew ;
		(*it)->m_histNew = 0;
		//it->weight = 0;
	}
	timeOver();
	pthread_mutex_unlock(&dwbuf_mutex);
}

int
bufManagerByDW::writeFileSegment(unsigned int fileId,unsigned int segId){
	if(lruQueue.size()==0){// the first block arrive
		myAlarmEvent.period = _period;
		myAlarmEvent.timeleft = _period;
		myAlarmEvent.callBackFunc = &DWBlockResetThread;
		myAlarmEvent.arg = this;
		addAlarmEvent(&myAlarmEvent);
	}
	pthread_mutex_lock(&dwbuf_mutex);
	if(lruQueue.size()>=_blockNum){
		pthread_mutex_unlock(&dwbuf_mutex);
		eliminateBlock();
	}else
		pthread_mutex_unlock(&dwbuf_mutex);
	addBlock(fileId,segId);
	return 0;
}

int
bufManagerByDW::readFileSegment(unsigned int fileId,unsigned int segId){
	list<DWBlockInfo*>::iterator it;
	DWBlockInfo* readedBlock;
	pthread_mutex_lock(&dwbuf_mutex);
	for(it = lruQueue.begin();it!=lruQueue.end();it++){
		//cout<<"block <"<<(*it)->fileId<<","<<(*it)->segId<<"> in "<<getClientId()<<endl;
		if( (*it)->fileId ==fileId && (*it)->segId == segId){
			(*it)->m_histNew ++;
			readedBlock = *it;
			lruQueue.erase(it);
			break;
		}
	}
	if(it == lruQueue.end()){
		pthread_mutex_unlock(&dwbuf_mutex);
		cout<<"in bufManagerByDW: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}else{
		lruQueue.push_back(readedBlock);
		pthread_mutex_unlock(&dwbuf_mutex);
	
	}
	//readBlock(fileId,segId);
	return 0;
}

int
bufManagerByDW::haveSegment(unsigned int fileId,unsigned int segId){
	list<DWBlockInfo*>::iterator it;
	pthread_mutex_lock(&dwbuf_mutex);
	for(it = lruQueue.begin();it !=lruQueue.end();it++){
		if((*it)->fileId == fileId && (*it)->segId == segId){
			pthread_mutex_unlock(&dwbuf_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&dwbuf_mutex);
	return false;
}
/*
int
bufManagerByDW::eliminateBlock(){
	unsigned int fileId,segId;
	unsigned int minWeight =111111111;
	DWBlockInfo* eliminateBlockPtr;
	list<DWBlockInfo*>::iterator it,minIt;
	pthread_mutex_lock(&dwbuf_mutex);
	for(it = lruQueue.begin();it!=lruQueue.end();it++){
		(*it)->weight = (*it)->m_histOld * 5 + (*it)->m_histNew * 6;
		if( (*it)->weight < minWeight){
			minWeight = (*it)->weight;
			eliminateBlockPtr = *it;
			minIt = it;
		}
	}
	//cout<<"delete "<< (*minIt)->fileId <<" "<<(*minIt)->segId<<endl;
	fileId = eliminateBlockPtr->fileId;
	segId = eliminateBlockPtr->segId;
	delete eliminateBlockPtr;
	lruQueue.erase(minIt);
	pthread_mutex_unlock(&dwbuf_mutex);
	sendEliminateBlock(fileId,segId);
	return 0;
}
*/
int
bufManagerByDW::eliminateBlock(){
	unsigned int fileId,segId;
	unsigned int minWeight =111111111;
	DWBlockInfo* eliminateBlockPtr;
	list<DWBlockInfo*>::iterator it,minHistIt,minHistNewIt,minIt;
	unsigned int index ,maxIndex,minHistNew;
	maxIndex = (int)(0.618 * _blockNum );
	index = 0;
	minHistNew = minWeight;
	pthread_mutex_lock(&dwbuf_mutex);
	for(it = lruQueue.begin();it!=lruQueue.end();it++){
		index ++;
		if(index >= maxIndex)
			break;
		(*it)->weight = (*it)->m_histOld * 5 + (*it)->m_histNew * 6;
		if(minHistNew > (*it)->m_histNew){
			minHistNewIt = it;
			minHistNew = (*it)->m_histNew;
		}
		if( (*it)->weight < minWeight){
			minWeight = (*it)->weight;
			minHistIt = it;
		}
		
	}
	if(minHistNew == 0){
		eliminateBlockPtr = *minHistNewIt;
		minIt = minHistNewIt;
	}else{
		eliminateBlockPtr = *minHistIt;
		minIt = minHistIt;
	}
	//cout<<"delete "<< (*minIt)->fileId <<" "<<(*minIt)->segId<<endl;
	fileId = eliminateBlockPtr->fileId;
	segId = eliminateBlockPtr->segId;
	delete eliminateBlockPtr;
	lruQueue.erase(minIt);
	pthread_mutex_unlock(&dwbuf_mutex);
	sendEliminateBlock(fileId,segId);
	return 0;
}

int
bufManagerByDW::addBlock(unsigned int fileId,unsigned int segId){
	if(haveSegment(fileId,segId)){
		cout<<"alread have the segment <"<<fileId<<","<<segId<<">"<<endl;
		return -1;
	}
	DWBlockInfo *temp = new DWBlockInfo(fileId,segId);
	pthread_mutex_lock(&dwbuf_mutex);
	lruQueue.push_back(temp);
	pthread_mutex_unlock(&dwbuf_mutex);
	sendAddedBlock(fileId,segId);
	return 0;
}
