#ifndef CLIENT_H
#define CLIENT_H

#include "../include/global.h"
#include "../include/message.h"
//#include "bufManager.h"
#include "bufManagerByLRU.h"
#include "bufManagerByLFU.h"
#include "bufManagerByDW.h"
#include "bufManagerByLFRU.h"
#include "bufManagerByLRFU.h"

#include "config.h"
#include "clientManager.h"

enum ClientStatus{
	STOP = 0,
	PAUSE,
	FORWARD,
	BACKWARD,
	PLAY
};

class ClientManager;

class Client{
	public:
		Client(unsigned int client_id,ClientConfigType *clientConfigInfo ,ClientManager *client_manager);
		~Client();
	
	public:
		int read_clientcircle_config(const string &fileName);
		int Run();

	public:
		int setRequestFile(unsigned int );
		unsigned int getRequestFileId(){return requestFileId;}
		int setMaxLoad(unsigned int);
		int setConfigFile(const string &file);
		int setClientStatus(unsigned int currentStatus);
		int getCurrentLoad();
		int getClientId()const ;
		int getPeriod(){
			return period;
		}
		int increaseCurrentLoad();
		int decreaseCurrentLoad();
		string getServerIp()const { return serverIp;}
		string getServerPort()const{ return serverPort;}
		int addAlarmEvent(TimeCallBack * callback);
	public:
		int playSegment(int sockfd,const unsigned int fileLen);
		int playingClient(int sockfd);
		int clientLifeCircle(int sockfd);
		//int getNextSegment(unsigned int &currentStatus);
		int getNextStatus(unsigned int &currentStatus);
		//send file segments request packet to server
		int requestFileSegments(int connfd,const unsigned int fileId,const unsigned int sidStart);
		//send  file segment request packet to  a client
		int requestFileSegment( unsigned int clientId,const unsigned int fileId, const unsigned int segId);
		//receive a request from a client
		int receiveRequest(const unsigned int fileId,const unsigned int segId,double bitRate);
		int sendExitMessage(int sockfd,unsigned int clientId);
		int sendAddedBlock(const unsigned int fileId,const unsigned int segId);
		int sendEliminateBlock(const unsigned int fileId,const unsigned int segId);
        int sendTimeTicks(double band);
        int sendCurrentLoad();
		int increaseBlockFromBuf();
		unsigned int getAvailableBand();
		unsigned int getBlockFromBuf();
		int increaseBlockFromClient();
		unsigned int getBlockFromClients();
		int increaseBlockFromServer();
		unsigned int getBlockFromServer();
		int decreaseAvailableBand(double bitRate);
		int increaseAvailableBand(double bitRate);
		string getBufRecordFile(){ return bufRecordFile;}
		bool needRecord(){return _needRecord;}
	private:
		unsigned int maxLoad;
		unsigned int clientId;
		unsigned int prefetchNumber;
		unsigned int bandwidth;
		string serverIp;
		string serverPort;
		string managerPort;
		string bufManagerStrategy;
		unsigned int blockSize;
		unsigned int totalBufSize;
        unsigned int managerSockfd;
		unsigned int period;
		float lrfuLambda;

	private:
		unsigned int playToPause;
		unsigned int playToPlay;
		unsigned int playToStop;
		unsigned int playToForward;
		unsigned int playToBackward;
		unsigned int pauseToPlay;
		unsigned int forwardToPlay;
		unsigned int backwardToPlay;
		double myzeta,mysigma;
		string configFile;
		string bufRecordFile;	
	private:
		bufManager * bufmanager;
		ClientManager *clientMaster;
	
	private:
		pthread_mutex_t client_mutex;
		unsigned int currentLoad;
		unsigned int clientStatus;
		unsigned int requestSegmentNum;
		unsigned int blockFromBuf;
		unsigned int blockFromClients;
		unsigned int blockFromServer;
		double availableBand;	
		bool _needRecord;
	private:
		unsigned int requestFileId;	
		unsigned int requestSegment;
		unsigned int requestFileLen;
		double requestFileBitRate;

};

#endif



