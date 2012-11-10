#ifndef BUFMANAGERBYLRU_H
#define BUFMANAGERBYLRU_H

#include "../include/global.h"
#include "bufManager.h"
#include <list>
using namespace std;

typedef struct blockInfo{
	unsigned int fileId;
	unsigned int segId;
}BlockInfo;

class bufManagerByLRU: public bufManager{
	public:
		bufManagerByLRU(Client*client,unsigned int blockSize,unsigned int totalBufSize);
		~bufManagerByLRU();
	public:
		 int writeFileSegment(unsigned int fileId,unsigned int segId);
		 int readFileSegment(unsigned int fileId,unsigned int segId);
		 int haveSegment(unsigned int fileId,unsigned int segId);
	public:
		 int eliminateBlock();
		 int addBlock(unsigned int fileId,unsigned int segId);
	private:
		 list<BlockInfo> buf;
		 pthread_mutex_t lru_mutex;
};

#endif
