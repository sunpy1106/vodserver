#ifndef BUFMANAGERBYLFU_H
#define BUFMANAGERBYLFU_H

#include "../include/global.h"
#include "bufManager.h"
#include <list>
using namespace std;

typedef struct LFUBlockInfo{
	unsigned int fileId;
	unsigned int segId;
	int counts;
}LFUBlockInfo;

class bufManagerByLFU: public bufManager{
	public:
		bufManagerByLFU(Client*client,unsigned int blockSize,unsigned int totalBufSize);
		~bufManagerByLFU();
	public:
		 int writeFileSegment(unsigned int fileId,unsigned int segId);
		 int readFileSegment(unsigned int fileId,unsigned int segId);
		 int haveSegment(unsigned int fileId,unsigned int segId);
	public:
		 int eliminateBlock();
		 int addBlock(unsigned int fileId,unsigned int segId);
	private:
		 list<LFUBlockInfo> buf;
		 pthread_mutex_t lfu_mutex;
		 int initial_parameter();

};

#endif
