#ifndef BUFMANAGERBYILRFU_H
#define BUFMANAGERBYILRFU_H

#include "../include/global.h"
#include "bufManager.h"
#include <list>
using namespace std;

struct timeval tv;
typedef struct blockInfo{
	unsigned int fileId;
	unsigned int segId;
	float weight;
	int lastTime;
	float lastWeight;
}BlockInfo;

typedef struct qOut{
	unsigned int fileId;
	unsigned int segId;
	float weight;
}QOut;

class bufManagerByILRFU: public bufManager{
	public:
		bufManagerByILRFU(Client*client,unsigned int blockSize,unsigned int totalBufSize):bufManager(client,blockSize,totalBufSize){}
		~bufManagerByILRFU();
	public:
		 int writeFileSegment(unsigned int fileId,unsigned int segId);
		 int readFileSegment(unsigned int fileId,unsigned int segId);
		 int haveSegment(unsigned int fileId,unsigned int segId);
	public:
		 int eliminateBlock();
		 int addBlock(unsigned int fileId,unsigned int segId);
	private:
		 list<BlockInfo> buf;
		 int initial_parameter();
		 float lambda;
		 int MODIFY_TIMES;								// 改变的时机
		 float MODIFY_RATIO;							// 改变的可能
		 float MODIFY_VALUE;							// 改变量
		 int inTimes;									// 缓冲区命中和新页面在Qout 中的次数
		 int outTimes;									// 新页面不在Qout 中的次数
		 int isPreIn;									// 上次的新页面是否在Qout 中, 为了统计连续出现的次数
		 int succTimes;									// 记录新页面连续在( 或不在) Qout 中出现的次数
		 int isInQout;									// 新页面是否在Qout 中或请求页面是否命中

};

#endif
