#ifndef BUFMANAGERBYLRFU_H
#define BUFMANAGERBYLRFU_H

#include "../include/global.h"
#include "bufManager.h"
#include <list>
using namespace std;

typedef struct lrfuBlockInfo{
	unsigned int fileId;
	unsigned int segId;
	float weight;
	int lastTime;
	float lastWeight;
}lrfuBlockInfo;

class bufManagerByLRFU: public bufManager{
	public:
		bufManagerByLRFU(Client*client,unsigned int blockSize,unsigned int totalBufSize,float lambda);
		~bufManagerByLRFU();
	public:
		 int writeFileSegment(unsigned int fileId,unsigned int segId);
		 int readFileSegment(unsigned int fileId,unsigned int segId);
		 int haveSegment(unsigned int fileId,unsigned int segId);
	public:
		 int eliminateBlock();
		 int addBlock(unsigned int fileId,unsigned int segId);
	private:
		 struct timeval tv;
		 list<lrfuBlockInfo> lrfuBuf;
		 int initial_parameter();
		 float _lambda;

};

#endif
