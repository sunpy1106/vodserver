#ifndef BUFMANAGERBYLFRU_H
#define BUFMANAGERBYLFRU_H

#include "../include/global.h"
#include "bufManager.h"
#include <list>
#include <sys/time.h>
using namespace std;

typedef struct LFRUBlockInfoo{
	unsigned int fileId;
	unsigned int segId;
	double weight;
	int periodCounter;
	unsigned long  lastAccessTime;
}LFRUBlockInfoo;


class bufManagerByLFRU: public bufManager{
	
	public:
		bufManagerByLFRU(Client*client,unsigned int blockSize,unsigned int totalBufSize,unsigned int period);
		~bufManagerByLFRU();
	public:
		int writeFileSegment(unsigned int fileId,unsigned int segId);
		int readFileSegment(unsigned int fileId,unsigned int segId);
		int haveSegment(unsigned int fileId,unsigned int segId);
	public:
		int eliminateBlock();
		int addBlock(unsigned int fileId,unsigned int segId);
		void blockReset();
	private:
		list<LFRUBlockInfoo> buf;
		unsigned long  recallTime;
		unsigned long t0;
		pthread_mutex_t  buf_mutex;
		unsigned int _period; 
		TimeCallBack myAlarmEvent;
};

#endif
