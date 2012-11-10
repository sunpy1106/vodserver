#ifndef SERVER_H
#define SERVER_H
#include "MYSQLPool.h"
#include "config.h"
#include "../include/global.h"
#include "../include/message.h"
#include <sys/epoll.h>

void *p2pServerThread(void *arg);
void *managerThread(void *arg);



class Server{
	public:
		Server(ServerConfigType *serverConfigInfo);
		~Server();

	public: //with the database
		int deleteAllResources();
		int insertResource(unsigned int fileId,unsigned int size,float bitRate);
		int getFileInfo(unsigned fileId,VideoInfo &info);
		int insertP2pClient(unsigned int id,unsigned int maxLoad);
		int updateP2pClient(unsigned int id,unsigned int currentLoad);
		int insertFileSegment(unsigned int fileId,unsigned int segId,unsigned clientId,unsigned int size);
		int deleteFileSegment(unsigned int fileId,unsigned int segId,unsigned clientId);
		int getFileSegInfo(unsigned int fileId,unsigned int segId,VodSeg &vod_seg);
		int resetClient();
		int resetClientSegment();
		int deleteClient(unsigned int clientId);
		int getAvailableBand();
		unsigned long getRequestedSegments();
		unsigned long  getServerProvideSegments();

	public:
		string getServerPort();
		unsigned int getCurrentLoad();
		void increaseCurrentLoad();
		void decreaseCurrentLoad();
		int increaseAvailableBand(double band);
		int decreaseAvailableBand(double band);
		unsigned int getMaxLoad();
		unsigned int getMinLoad();
		int getRefusedClients();
		int printDataMember();
		int clientRequestSegment();
		int serverProvideSegment();
	public://server the client
		int initServer();
		int startVodServer();
		int doStatistical();
		int receiveMessage(int sockfd);
		int startClientManager();
		int addNewClient(int epfd,int listenfd);
		int readPacket(int connfd);
		int messageProcess(int connfd,void * buf,size_t messageLen);
		int sendMessage(int connfd,void * buf,int len);
		int sendTimeTicks(unsigned int size,unsigned int clientId);
		void printStartedUser();
	private:
		ServerConfigType *serverConfig;
		MYSQLPool *pool;
		bool isP2pOpen;
		bool startedStatistical ;
		string serverPort;
		string managerPort;
		unsigned int clientNumber;
		bool startRecord;
		unsigned int blockSize;
	private:
		unsigned int bandwidth;
		double availableBandwidth;
		unsigned int maxLoad;
		unsigned int minLoad;
		unsigned int currentLoad;
		unsigned int refusedClients;
		pthread_mutex_t ser_mutex;
		unsigned long requestedSegments;
		unsigned long serverProvideSegments;
//		unsigned int blockSize ;
		unsigned int startedUser;	
};

class reqHandlerArg{
	public:
		reqHandlerArg(Server *server,int sockfd){
			_server = server;
			_sockfd = sockfd;
		}
		Server *_server;
		int _sockfd;
};

#endif

