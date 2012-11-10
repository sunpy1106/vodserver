#include "server.h"

#define BUF_SIZE 1024
#define sendTimeOutSec 30
#define recvTimeOutSec 30

void *p2pServerThread(void *arg){
	Server *realServer = (Server*)arg;
	realServer->startVodServer();
	return NULL;
}

void *managerThread(void *arg){
	Server *realServer = (Server*)arg;
	realServer->startClientManager();
	return NULL;
}

void *statisticalThread(void *arg){
	Server *realServer = (Server*)arg;
	realServer->doStatistical();
	return NULL;
}

Server::Server(ServerConfigType *serverConfigInfo){
	serverConfig = serverConfigInfo;
	pool = new MYSQLPool(serverConfig->dbFileName.c_str());
	currentLoad = refusedClients = 0;
	isP2pOpen = serverConfig->isP2pOpen;
	maxLoad = 0;
	minLoad = 10000;
	bandwidth = serverConfig->bandwidth;
	availableBandwidth =(double) bandwidth;
//	clientBandwidth = serverConfig->clientBandwidth;
	clientNumber = serverConfig->clientNumber;
	blockSize = 10;
	startRecord = false;
	startedStatistical = false;
	requestedSegments =0;
	startedUser=0;
	serverProvideSegments = 0;
	serverPort = serverConfig->serverPort;
	managerPort = serverConfig->managerPort;
	pthread_mutex_init(&ser_mutex,NULL);
	//buf = (char*)malloc(sizeof(char) * BUF_SIZE);
	//InitServer();
}

Server::~Server(){
	delete pool;
	//free(buf);
	pthread_mutex_destroy(&ser_mutex);
}

int
Server::initServer(){
	unsigned int resourceNum = serverConfig->resourceNumber;
	unsigned int minBitRate = serverConfig->minBitRate;
	unsigned int maxBitRate = serverConfig->maxBitRate;
	unsigned int minLen = serverConfig->minLen;
	unsigned int maxLen = serverConfig->maxLen;
	double bitRate ;
	unsigned int fileLen;
	if(serverConfig->needCreateResources == true){
		deleteAllResources();
		for(size_t i = 0;i<resourceNum;i++){
			bitRate = Randomf(minBitRate,maxBitRate);
			fileLen = Randomi(minLen,maxLen);
			cout<<i<<"\t"<<fileLen<<"\t"<<bitRate<<endl;
			insertResource(i,fileLen,bitRate);
		}
	}
	resetClient();
	resetClientSegment();
	pthread_t tid1;
	//create the p2p server thread
	pthread_create(&tid1,NULL,p2pServerThread,(void*)this);
	pthread_detach(tid1);
	//create the manager thread
	pthread_t tid2;
	pthread_create(&tid2,NULL,managerThread,(void*)this);
	pthread_detach(tid2);
	pthread_t tid3;
	startedStatistical = true;
	pthread_create(&tid3,NULL,statisticalThread,(void*)this);
	pthread_detach(tid3);
    for(;;)
        pause();
    return 0;
}


int
Server::clientRequestSegment(){
	pthread_mutex_lock(&ser_mutex);
	requestedSegments ++;
	pthread_mutex_unlock(&ser_mutex);
	return 0;
}
void
Server::printStartedUser(){
	cout<<"startedUser = "<<startedUser<<endl;
}
int
Server::serverProvideSegment(){
	pthread_mutex_lock(&ser_mutex);
	serverProvideSegments ++;
	pthread_mutex_unlock(&ser_mutex);
	return 0;
	
}

void 
Server::increaseCurrentLoad(){
	pthread_mutex_lock(&ser_mutex);
	currentLoad ++;
	if(currentLoad > maxLoad && startRecord == true)
		maxLoad = currentLoad;
	pthread_mutex_unlock(&ser_mutex);
}

void
Server::decreaseCurrentLoad(){
	pthread_mutex_lock(&ser_mutex);
	currentLoad --;
	if(currentLoad < minLoad && startRecord == true){
		minLoad = currentLoad;
	}
	pthread_mutex_unlock(&ser_mutex);
}

int
Server::decreaseAvailableBand(double band){
	int retcode;
	pthread_mutex_lock(&ser_mutex);
	if(availableBandwidth < band){
		retcode = -1;
	}else{
		availableBandwidth -= band;
		retcode = 0;
	}
	pthread_mutex_unlock(&ser_mutex);
	return retcode;
}

int
Server::increaseAvailableBand(double band){
	pthread_mutex_lock(&ser_mutex);
	availableBandwidth += band;
	if((unsigned int)availableBandwidth >bandwidth){
		cout<<"program error increase bandwidth"<<endl;
		exit(1);
	}
	pthread_mutex_unlock(&ser_mutex);
	return 0;
	
}
unsigned long
Server::getRequestedSegments(){
	unsigned long result;
	pthread_mutex_lock(&ser_mutex);
	result = requestedSegments ;
	pthread_mutex_unlock(&ser_mutex);
	return result;
}

unsigned long
Server::getServerProvideSegments(){
	unsigned long result;
	pthread_mutex_lock(&ser_mutex);
	result = serverProvideSegments ;
	pthread_mutex_unlock(&ser_mutex);
	return result;
}

unsigned int 
Server::getMaxLoad(){
	unsigned int result;
	pthread_mutex_lock(&ser_mutex);
	result = maxLoad;
	pthread_mutex_unlock(&ser_mutex);
	return result;
}

unsigned int
Server::getMinLoad(){
	unsigned int result;
	pthread_mutex_lock(&ser_mutex);
	result = minLoad;
	pthread_mutex_unlock(&ser_mutex);
	return result;
}

int 
Server::doStatistical(){
	//statistics
	ofstream ofs;
	ofs.open((serverConfig->statisticalResultFile).c_str());
	if(isP2pOpen == true){
		ofs<<"P2p is used :"<<endl;
		ofs<<"currntLoad\thitRate\t\tminLoad\t\tmaxLoad"<<endl;
	}else{
		ofs<<"p2p is not used :"<<endl;
		ofs<<"currentLoad \trefusedClients\tminLoad\tmaxLoad"<<endl;
	}
	unsigned long nowTotal,nowClients;
	for(;;){
		if(isP2pOpen == true){
			ofs <<getCurrentLoad()<<"\t\t";
//			ofs <<getTotalLoad()<<"\t\t";
//			ofs <<getRefusedClients()<<"\t\t\t";
			nowTotal  = getRequestedSegments();
			nowClients = nowTotal -  getServerProvideSegments();
			if(nowTotal ==0)
				ofs<<"0"<<"  ";
			else
				ofs<<nowClients/(double)nowTotal<<"  ";
			if(startRecord == true){
				ofs<<getMinLoad()<<"  ";
				ofs<<getMaxLoad();
			}
			ofs<<endl;
			ofs.flush();
		}else{
			ofs << getCurrentLoad()<<"\t"<<getRefusedClients()<<endl;
			if(startRecord == true)
				ofs<<getMinLoad()<<"\t"<<getMaxLoad()<<"\t"<<endl;
			ofs.flush();
		}
		sleep(serverConfig->statisticalTimeInterval);
	}
	ofs.close();
	return 0;
}

string 
Server::getServerPort() {
	return serverPort;
}


int
Server::addNewClient(int epfd,int listenfd){
	struct epoll_event ev;
	struct sockaddr_in cliaddr;
	int len  = sizeof(cliaddr);
	int connfd =  accept(listenfd,(struct sockaddr *)&cliaddr,(socklen_t*)&len);
	if(connfd <0){
		perror("accept error!");
		return -1;
	}
	ev.data.fd = connfd;
	ev.events = EPOLLIN;
	epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
	return connfd;
}

int 
Server::readPacket(int connfd){
	char buf[BUF_SIZE];
	int len = recv(connfd,(void*)buf,BUF_SIZE,0);
	if(len <0){
		perror("recv error");
		return -1;
	}
	else if( len ==0){
		return 0;
	}else{
		messageProcess(connfd,buf,len);
		return 1;
	}
}


int
Server::startClientManager(){
	int listen_fd = tcp_listen(managerPort.c_str());
	int clientSize = 1000;
	int epfd,i;
	struct epoll_event listen_ev,events[clientSize];
	epfd  = epoll_create(10);
	listen_ev.events = EPOLLIN;
	listen_ev.data.fd  = listen_fd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,listen_fd,&listen_ev);
	int nready;
	for(;;){
		nready = epoll_wait(epfd,events,clientSize,-1);
		if(nready < 0){
			if(errno == EINTR)
				continue;
			perror("epoll_wait error");
			exit(1);
		}
		for(i= 0;i<nready;i++){
			if(events[i].data.fd == listen_fd){
				addNewClient(epfd,listen_fd);
			}else if(events[i].events && EPOLLIN){
				int flag = readPacket(events[i].data.fd);
				if(flag == 0){
					close(events[i].data.fd);
					epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,events + i );
				}
			}
			else if(events[i].events && EPOLLERR){
				close(events[i].data.fd);
				epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,events+i);
			}
		}
	}
	return 0;		
}



void * reqHandler(void *arg){
	reqHandlerArg *reqHandler = (reqHandlerArg*)arg;
	Server *server = reqHandler->_server;
	server->receiveMessage(reqHandler->_sockfd);
	return NULL;
}

int
Server::receiveMessage(int sockfd){
	char buf[BUF_SIZE];
	int nread;
	struct timeval timeout;
	fd_set rset;
	FD_ZERO(&rset);
	for(;;){
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000;
		nread = 0;
		FD_SET(sockfd,&rset);
	    timeout.tv_sec = recvTimeOutSec;
	    timeout.tv_usec = 0;
		int ret = select(sockfd + 1,&rset,NULL,NULL,&timeout);
		if(ret < 0){
			if(errno == EINTR)
				continue;
			perror("select error in recv message");
			return -1;
		}
		if(FD_ISSET(sockfd,&rset)){
			nread = recv(sockfd,buf,BUF_SIZE,0);
			if(nread < 0){
				perror("in receive message thread, recv error");
				return -1;
			}
			else if(nread ==0){
				//cout<<"client connection closed"<<endl;
				close(sockfd);
				//decreaseCurrentLoad();
				return 0;
			}else{
				messageProcess(sockfd,buf,nread);
			}
		}
		memset(buf,'\0',BUF_SIZE);	
	}
//	free(buf);
	return 0;
}



int
Server::startVodServer(){
	int listen_fd = tcp_listen(serverPort.c_str());
	int sockfd;
	pthread_t tid;
	struct sockaddr_in cliaddr;
	socklen_t cliLen = sizeof(cliaddr);
	for(;;){
		sockfd = accept(listen_fd,(struct sockaddr*)&cliaddr,&cliLen);
		if(sockfd < 0){
			perror("accept a client connection error");
			continue;
		}else{
/*			startedUsers++;
			curLoad = getCurrentLoad();
			if(curLoad>= maxLoad){
				cout<<"too much load,curLoad = "<<getCurrentLoad()<<",maxLoad = "<<maxLoad<<endl;
				cout<<"************************now started users = "<<startedUsers<<endl;
				exit(1);
				pthread_mutex_lock(&ser_mutex);
				refusedClients ++;
				pthread_mutex_unlock(&ser_mutex);
				close(sockfd);
			}else{
				reqHandlerArg *arg = new reqHandlerArg(this,sockfd);
				pthread_create(&tid,NULL,reqHandler,(void*)arg);
				pthread_detach(tid);
			}
*/
			startedUser++;
			if(startedUser >= clientNumber){
				startRecord = true;
			}
			reqHandlerArg *arg = new reqHandlerArg(this,sockfd);
			pthread_create(&tid,NULL,reqHandler,(void*)arg);
			pthread_detach(tid);

		}
/*
		if(startedUsers >= serverConfig->clientNumber && (startedStatistical== false)){
			if(isP2pOpen == false)
				continue;
			startedStatistical = true;
			pthread_t tid3;
			pthread_create(&tid3,NULL,statisticalThread,(void*)this);
			pthread_detach(tid3);
		}
*/		
	}
	return 0;
}
int
Server::sendMessage(int cliSockfd,void* buf,int len){
    unsigned int bytesLen = len;
    unsigned int nsent = 0, offset = 0;
    struct timeval timeout;
    timeout.tv_sec = sendTimeOutSec;
    timeout.tv_usec = 0;

    if (cliSockfd < 0 ) {
        cout << "Error : sockfd < 0 " << endl;
        return -1;
    }

    fd_set wset;
    FD_ZERO( &wset );

    while (true ) {
        // need to check the socket status, before set the FDSET
        // if the socket is not available, return ..
        FD_SET( cliSockfd, &wset );

        int ret = select(cliSockfd + 1, NULL, &wset, NULL, &timeout);
		if(ret < 0){
			perror("select error in send message");
			return -1;
		}
		if (FD_ISSET( cliSockfd, &wset )) {
            //nsent = send(cliSockfd, (char *)buf + offset, bytesLen, MSG_NOSIGNAL);
            nsent = send(cliSockfd, (char *)buf + offset, bytesLen, 0);

            if ( nsent < 0) {
                cout << "\tError : SEND_FULL_PACK " 
                    << strerror(errno) << endl;
                return -1;
            } else if (nsent == 0) {
                cout << "\tError : send return zero, connection closed?"  << endl
                     << strerror(errno) << endl;
                return -1;
            } else {
                offset += nsent;   // already sent
                bytesLen -= nsent; // the left bytes for send 
                if (offset == (unsigned int)len ) { // just send all pack
                    break;
                } 
            }
        }
	}

	return 0;

}

int
Server::sendTimeTicks(unsigned int size,unsigned int clientId){
	unsigned int sleepTime;
	unsigned int fileLen ;
	float band;
	pool->getFileInfo(clientId,fileLen,band);
	if(decreaseAvailableBand(band)<0){
		cout<<"too much load"<<endl;
		printStartedUser();
		exit(1);
	}
	increaseCurrentLoad();
	sleepTime= 8000000*size /band;
	cout<<"sleepTime = "<<sleepTime<<",size = "<<size<<",band = "<<band<<endl;
	mysleep(sleepTime);
	decreaseCurrentLoad();
	increaseAvailableBand(band);
	return 0;
}
int
Server::messageProcess(int connfd,void *buf,size_t bufLen){
	VideoInfo info ;
	VideoInfoReq *clientInfo;
	vodSegReq *vsr;
	VodSeg *vsa;
	ClientState *cs;
	BufUpdatePack *packPtr;
	int messageType = ntohl(*((int *)buf));
//	cout<<"messageType = "<<messageType<<endl;
	switch(messageType){
		case REQUESTRESOURCE:
			cout<<"receive the REQUESTRESOURCE message"<<endl;
			cout<<"bufLen ="<<bufLen<<endl;
			cout<<"sizeof(VideoInfoReq) = "<<sizeof(VideoInfoReq)<<endl;
			if(bufLen < (int)sizeof(VideoInfoReq)){
				cout<<"recv error in recv REQUESTRESOURCE message"<<endl;
				perror("recv error");
				exit(1);
			}
			clientInfo = (VideoInfoReq*)buf;
			cout<<"receive REQUESTSOURCE message from "<<ntohl(clientInfo->clientId)<<endl;
			getFileInfo(ntohl(clientInfo->resourceId),info);
			if(isP2pOpen == true){
				insertP2pClient(ntohl(clientInfo->clientId),ntohl(clientInfo->bandwidth));
			}
			sendMessage(connfd,(void*)&info,sizeof(VideoInfo));
			cout<<"send  REQUESTSOURCE message to "<<ntohl(clientInfo->clientId)<<endl;
			//cout<<" ,fileLen = "<<info.fileLength<<",bitRate = "<<info.bitRate<<endl;
			if(bufLen > sizeof(VideoInfoReq)){
				messageProcess(connfd,(char*)buf+sizeof(VideoInfoReq),bufLen - sizeof(VideoInfoReq));
			}
			break;
		case REQUESTSEGMENT:
			cout<<"receive the REQUESTSEGMENT message"<<endl;
			cout<<"bufLen = "<<bufLen <<endl;
			cout<<"sizeof vodSegReq = "<<sizeof(VodSegReq)<<endl;
			if(bufLen < sizeof(VodSegReq)){
				cout<<"recv error in recv REQUESTSEGMENT message"<<endl;
				perror("recv error");
				exit(1);
			}
			vsr = (VodSegReq *)buf;
			vsa =  (VodSeg*)malloc(sizeof(VodSeg));
			for(size_t i= 0;i<ntohl(vsr->sidNum);i++){
				cout<<"client "<<ntohl(vsr->clientId)<<" request  segment < "<<ntohl(vsr->resourceId)<<","<<ntohl(vsr->sidStart) + i <<"> from server"<<endl; 
				if(startedStatistical == true && isP2pOpen == true){
					clientRequestSegment();
				}
				if(isP2pOpen == false){
					//can't find that kind of client
					vsa->resourceId = vsr->resourceId;
					vsa->sid = htonl(ntohl(vsr->sidStart) + i);
					vsa->clientId = htonl(SERVERID);
					cout<<"send segment < "<<ntohl(vsr->resourceId)<<","<<ntohl(vsr->sidStart) + i <<"> to client "<<ntohl(vsr->clientId)<<endl;
					sendMessage(connfd,(void*)vsa,sizeof(VodSeg) );
					sendTimeTicks(blockSize,ntohl(vsr->clientId));	
				}else{
					if((getFileSegInfo(ntohl(vsr->resourceId),ntohl(vsr->sidStart) + i,*(vsa)) <0)){
						cout<<"can't find the segment in other client,the server will provide it"<<endl;
						vsa->resourceId = vsr->resourceId;
						vsa->sid = htonl(ntohl(vsr->sidStart) + i);
						vsa->clientId = htonl(SERVERID);
						if(startedStatistical == true && isP2pOpen == true){
							serverProvideSegment();
						}
						cout<<"send segment < "<<ntohl(vsr->resourceId)<<","<<ntohl(vsr->sidStart) + i <<"> to client "<<ntohl(vsr->clientId)<<endl;
					
						sendMessage(connfd,(void*)vsa,sizeof(VodSeg) );
						sendTimeTicks(blockSize,ntohl(vsr->clientId));	
					}else{
						cout<<"find the segment in client "<<ntohl(vsa->clientId)<<endl;
						cout<<"send segment < "<<ntohl(vsr->resourceId)<<","<<ntohl(vsr->sidStart) + i <<"> to client "<<ntohl(vsr->clientId)<<endl;
					
						sendMessage(connfd,(void*)vsa,sizeof(VodSeg) );
						
					}
				}
			}
			if(bufLen > sizeof(VodSegReq)){
				messageProcess(connfd,(char*)buf+sizeof(VodSegReq),bufLen - sizeof(VodSegReq));
			}
			free(vsa);
			break;
		case REQUESTSEGMENTAGAIN:
			cout<<"receive the REQUESTSEGMENTAGAIN message"<<endl;
			cout<<"bufLen = "<<bufLen <<endl;
			cout<<"sizeof vodSegReq = "<<sizeof(VodSegReq)<<endl;
			if(bufLen < sizeof(VodSegReq)){
				cout<<"recv error in recv REQUESTSEGMENT message"<<endl;
				exit(1);
			}
			vsr = (VodSegReq *)buf;
			vsa =  (VodSeg*)malloc(sizeof(VodSeg));
			for(size_t i= 0;i<ntohl(vsr->sidNum);i++){
				cout<<"client "<<ntohl(vsr->clientId)<<" request  segment < "<<ntohl(vsr->resourceId)<<","<<ntohl(vsr->sidStart) + i <<"> from server"<<endl; 
				vsa->resourceId = vsr->resourceId;
				vsa->sid = htonl(ntohl(vsr->sidStart) + i);
				vsa->clientId = htonl(SERVERID);
				cout<<"******************8888the clientId = "<<ntohl(vsa->clientId)<<endl;
				if(startedStatistical == true && isP2pOpen == true){

					serverProvideSegment();
				}
				cout<<"send segment < "<<ntohl(vsa->resourceId)<<","<<ntohl(vsa->sid) <<"> to client "<<ntohl(vsr->clientId)<<endl;	
				sendMessage(connfd,(void*)vsa,sizeof(VodSeg) );
				sendTimeTicks(blockSize,ntohl(vsr->clientId));	
			}
			free(vsa);
			if(bufLen > sizeof(VodSegReq)){
				messageProcess(connfd,(char*)buf+sizeof(VodSegReq),bufLen - sizeof(VodSegReq));
			}
			break;
		
		
		case HEARTBEAT:
			cout<<"receive the HEARTBEAT message"<<endl;
			cout<<"bufLen = "<<bufLen <<endl;
			cout<<"sizeof clientState = "<<sizeof(ClientState)<<endl;
			if(bufLen <  sizeof(ClientState)){
				cout<<"recv error in recv HEART BEAT message"<<endl;
				perror("recv error");
				exit(1);
			}
			cs = (ClientState*)buf;
			updateP2pClient(ntohl(cs->clientId),ntohl(cs->availableBand));
			if(bufLen > sizeof(VodSegReq)){
				messageProcess(connfd,(char*)buf+sizeof(VodSegReq),bufLen - sizeof(VodSegReq));
			}
			break;
		case BLOCKADD:
			if(isP2pOpen ==false){
				break;
			}
			cout<<"receive the blockAdd messsage"<<endl;
			packPtr = (BufUpdatePack*)buf;
			cout<<"bufLen = "<<bufLen <<endl;
			cout<<"sizeof BufUpdatePack = "<<sizeof(BufUpdatePack)<<endl;
			if(bufLen < sizeof(BufUpdatePack)){
				cout<<"recv error in recv blockadd message"<<endl;
				exit(1);
			}
			cout<<"receive the blockadd message,add <"<<ntohl(packPtr->fileId)<<","<<ntohl(packPtr->segId)<<"> from client "<<ntohl(packPtr->clientId)<<endl;
			insertFileSegment(ntohl(packPtr->fileId),ntohl(packPtr->segId),ntohl(packPtr->clientId),ntohl(packPtr->blockSize));
			if(bufLen > sizeof(BufUpdatePack)){
				messageProcess(connfd,(char*)buf+sizeof(BufUpdatePack),bufLen - sizeof(BufUpdatePack));
			}
			break;
		case BLOCKELIMINATE:
			if(isP2pOpen ==false){
				break;
			}
			cout<<"receive the block eliminate messsage"<<endl;
			packPtr = (BufUpdatePack*)buf;
			cout<<"bufLen = "<<bufLen <<endl;
			cout<<"sizeof BufUpdatePack = "<<sizeof(BufUpdatePack)<<endl;
			if(bufLen < sizeof(BufUpdatePack)){
				cout<<"recv error in recv blockeliminate message"<<endl;
				exit(1);
			}
			deleteFileSegment(ntohl(packPtr->fileId),ntohl(packPtr->segId),ntohl(packPtr->clientId));	
			if(bufLen > sizeof(BufUpdatePack)){
				messageProcess(connfd,(char*)buf+sizeof(BufUpdatePack),bufLen - sizeof(BufUpdatePack));
			}
			break;
		case CLIENTQUIT:
			cout<<"receive the clientquit message"<<endl;
			cout<<"bufLen = "<<bufLen <<endl;
			cout<<"sizeof clientState = "<<sizeof(ClientState)<<endl;
			if(bufLen <  sizeof(ClientState)){
				cout<<"recv error in recv clientQuit message"<<endl;
				exit(1);
			}
			if(isP2pOpen== true){
				cs = (ClientState*)buf;
				deleteClient(ntohl(cs->clientId));	
			}
			if(bufLen > sizeof(ClientState)){
				messageProcess(connfd,(char*)buf+sizeof(ClientState),bufLen - sizeof(ClientState));
			}
			close(connfd);
			break;
		default:
			cout<<"wrong message"<<endl;
			break;
	}
	return 0;
}

unsigned int
Server::getCurrentLoad(){
	unsigned int result ;
	pthread_mutex_lock(&ser_mutex);
	result = currentLoad;
	pthread_mutex_unlock(&ser_mutex);
	return result;
}

int
Server::getRefusedClients(){
	int result;
	pthread_mutex_lock(&ser_mutex);
	result = refusedClients;
	pthread_mutex_unlock(&ser_mutex);
	return result;
}

int
Server::deleteAllResources(){
	pool->deleteAllResources();
	return 0;
}

int
Server::insertResource(unsigned int fileId,unsigned int fileSize,float bitRate){
	pool->insertResource(fileId,fileSize,bitRate);
	return 0;
}



int 
Server::getFileInfo(unsigned int fileId,VideoInfo &info){
	unsigned int fileLen;
	float bitRate;
	pool->getFileInfo(fileId,fileLen,bitRate);
	info.resourceId = htonl(fileId);
	info.fileLength = htonl(fileLen);
	info.bitRate = bitRate;
	return 0;
}



int 
Server::insertP2pClient(unsigned int id,unsigned int maxLoad){
	unsigned int availableBand = maxLoad;
	pool->insertP2pClient(id,maxLoad,availableBand);
	return 0;
}
		
int 
Server::updateP2pClient(unsigned int id,unsigned int availableBand){
	pool->updateP2pClient(id,availableBand);
	return 0;
}

int
Server::insertFileSegment(unsigned int fileId,unsigned int segId,
				unsigned clientId,unsigned int size){
	pool->insertFileSegment(fileId,segId,clientId,size);
	return 0;
}

int
Server::deleteFileSegment(unsigned int fileId,unsigned int segId,unsigned clientId){
	pool->deleteFileSegment(fileId,segId,clientId);
	
	return 0;
}
int
Server::getFileSegInfo(unsigned int fileId,unsigned int segId,VodSeg &vod_seg){
	unsigned int clientId;
	if(pool->getFileSegInfo(fileId,segId,clientId)<0)
		return -1;
	vod_seg.resourceId = htonl(fileId);
	vod_seg.sid = htonl(segId);
	vod_seg.clientId = htonl(clientId);
	return 0;
}

int
Server::resetClient(){
	pool->resetClient();
	return 0;
}

int
Server::resetClientSegment(){
	pool->resetClientSegment();
	return 0;
}
int
Server::deleteClient(unsigned int clientId){
	pool->deleteClient(clientId);
	return 0;
}

int
Server::getAvailableBand(){
	return pool->getAvailableBand();
}


int
Server::printDataMember(){
	cout<<"isP2pOpen = "<<((isP2pOpen==true )? "true":"false")<<endl;
	cout<<"maxLoad = "<<maxLoad<<endl;
	cout<<"currentLoad = "<<currentLoad<<endl;
	cout<<"refusedClients = "<<refusedClients<<endl;
	cout<<"serverPort = "<<serverPort<<endl;
	return 0;
}

