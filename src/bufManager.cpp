#include "bufManager.h"
#include "client.h"

const unsigned int debugClient = 0;

bufManager::bufManager(Client *client,unsigned int blockSize,unsigned int totalBufSize){
	owner  = client;
	_blockSize = blockSize;
	_totalBufSize = totalBufSize;
	_blockNum = (totalBufSize%blockSize ==0)?(totalBufSize/blockSize):(totalBufSize/blockSize + 1);
	_clientId = client->getClientId();
	bufRecordFile = client->getBufRecordFile();
	int j= bufRecordFile.find('.');
	string bufRecord=(bufRecordFile).substr(0,j) + numToString(_clientId) + bufRecordFile.substr(j);
	_needRecord = client->needRecord();
	if(_needRecord == true){
		bufOfs.open(bufRecord.c_str(),ifstream::in | ifstream::trunc);
		if(bufOfs.is_open() == false){
			cout<<"open the file error"<<endl;
			exit(1);
		}
		pthread_mutex_init(&_osmutex,NULL);
	}
}

bufManager::~bufManager(){
	if(_needRecord == true){
		bufOfs.close();
		pthread_mutex_destroy(&_osmutex);
	}
}
int
bufManager::timeOver(){
	if(_needRecord == false)
		return -1;
	pthread_mutex_lock(&_osmutex);
	bufOfs<<"0\t0\tReset"<<endl; 
	pthread_mutex_unlock(&_osmutex);
	return 0;
}

void
bufManager::blockReset(){
}

int 
bufManager::readBlock(unsigned int fileId,unsigned int segId){
	if(_needRecord==true){
		pthread_mutex_lock(&_osmutex);
		bufOfs<<fileId<<"\t"<<segId<<"\t"<<"Read"<<endl;
		pthread_mutex_unlock(&_osmutex);
	}
	return 0;
}

int 
bufManager::sendAddedBlock(unsigned int fileId,unsigned int segId){
	if( _needRecord == true){
		pthread_mutex_lock(&_osmutex);
		bufOfs<<fileId<<"\t"<<segId<<"\t"<<"ADD"<<endl;
		pthread_mutex_unlock(&_osmutex);
	}
	owner->sendAddedBlock(fileId,segId);
	return 0;
}
int
bufManager::sendEliminateBlock(unsigned int fileId,unsigned int segId){
	if(_needRecord == true){
		pthread_mutex_lock(&_osmutex);
		bufOfs<<fileId<<"\t"<<segId<<"\t"<<"Eliminate"<<endl;
		pthread_mutex_unlock(&_osmutex);
	}
	owner->sendEliminateBlock(fileId,segId);
	return 0;
}
int
bufManager::addAlarmEvent(TimeCallBack * callBack){
	owner->addAlarmEvent(callBack);
	return 0;
}

int
bufManager::getClientId()const{
	return owner->getClientId();
}
