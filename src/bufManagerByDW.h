#ifndef BUFMANAGERBYDW_H
#define BUFMANAGERBYDW_H

#include "../include/global.h"
#include "bufManager.h"
#include <list>
#include <signal.h>

using namespace std;

class DWBlockInfo{
	public:
	DWBlockInfo(unsigned int _fileId,unsigned int _segId):fileId(_fileId),segId(_segId){
		weight = 0;
		m_histNew = 0;
		m_histOld = 0;
	}
	unsigned int fileId;
	unsigned int segId;
	unsigned int weight;
	unsigned int  m_histNew;
	unsigned int  m_histOld;
};



class bufManagerByDW: public bufManager{
	public:
		bufManagerByDW(Client*client,unsigned int blockSize,unsigned int totalBufSize,unsigned int period);
		~bufManagerByDW();
	public:
		 int writeFileSegment(unsigned int fileId,unsigned int segId);
		 int readFileSegment(unsigned int fileId,unsigned int segId);
		 int haveSegment(unsigned int fileId,unsigned int segId);
	public:
		 void blockReset();
	public:
		 int eliminateBlock();
		 int addBlock(unsigned int fileId,unsigned int segId);
	private:
	//	 list<DWBlockInfo> dwBuf;
		 list<DWBlockInfo*> lruQueue;
		 pthread_mutex_t dwbuf_mutex;
		 unsigned int _period;
		 TimeCallBack myAlarmEvent;

};

#endif
