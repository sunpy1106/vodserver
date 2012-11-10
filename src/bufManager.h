#ifndef	BUFMANAGER_H
#define BUFMANAGER_H

#include "../include/global.h"
//#include "client.h"

class Client;

class bufManager{
	public:
		bufManager(Client *client,unsigned int blockSize,unsigned int totalBufSize);
		~bufManager();
	public://seven manager strategies that must be implement
		virtual int writeFileSegment(unsigned int fileId,unsigned int segId)=0;
		virtual int readFileSegment(unsigned int fileId,unsigned int segId)=0;
	public:
		virtual int eliminateBlock() =0;
		virtual int addBlock(unsigned int fileId,unsigned int segId) = 0;
		virtual int haveSegment(unsigned int fileId,unsigned int segId)= 0;
		virtual void blockReset();
		int sendEliminateBlock(unsigned int fileId,unsigned int segId);
		int addAlarmEvent(TimeCallBack * callBack);
		int sendAddedBlock(unsigned int fileId,unsigned int segId);
		int readBlock(unsigned int fileId,unsigned int segId);
		int timeOver();
		int getClientId()const;
	public:
		 Client *owner;
		 unsigned int _blockSize;
		 unsigned int _totalBufSize;
		 unsigned int _blockNum;
		 unsigned int _clientId;
	public:
		 pthread_mutex_t _osmutex;
		 ofstream bufOfs;		
		 bool _needRecord;
		 string bufRecordFile;
};
#endif
