#ifndef BUFMANAGERBYLRU_H
#define BUFMANAGERBYLRU_H

#include "../include/global.h"
#include "bufManager.h"
#include <list>
#define T 500000
using namespace std;

struct timeval tv;
typedef struct blockInfo{
	unsigned int fileId;
	unsigned int segId;
	float weight;
	int Ck;								//×Ü¼ÆÊý
	int Ck_1;							//Fki
	int Ck_2;							//Fki-1
	int Ck_3;							//Fki-2
	int periodCounter;
}BlockInfo;

class bufManagerByPLFU: public bufManager{
	public:
		bufManagerByPLFU(Client*client,unsigned int blockSize,unsigned int totalBufSize):bufManager(client,blockSize,totalBufSize){}
		~bufManagerByPLFU();
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
		 int sig_alarm(int signo);
		 int t0;

};

#endif
