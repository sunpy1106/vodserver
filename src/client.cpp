#include "client.h"

#define BUFSIZE 1024
#define JUMPMINUTES 1

Client::Client(unsigned int client_id,ClientConfigType *clientConfigInfo,ClientManager *client_manager){
	clientId = client_id;
	prefetchNumber = clientConfigInfo->prefetchNumber;
	serverIp.assign(clientConfigInfo->serverIpAddress);
	serverPort.assign(clientConfigInfo->serverPort);
	managerPort.assign(clientConfigInfo->managerPort);
	bufManagerStrategy = clientConfigInfo->bufManagerStrategy;
	blockSize = clientConfigInfo->blockSize;
	period = clientConfigInfo->period;
	bandwidth = clientConfigInfo->clientBandwidth;
	lrfuLambda = clientConfigInfo->lrfuLambda;
	totalBufSize = clientConfigInfo->totalBufSize;
	currentLoad =0;
    _needRecord = false;
	bufRecordFile = clientConfigInfo->bufRecordFile;
	maxLoad =0;
	availableBand = (double)bandwidth;
	blockFromBuf = blockFromClients = blockFromServer = requestSegmentNum = 0;
	pthread_mutex_init(&client_mutex,NULL);
	clientMaster = client_manager;
	if(bufManagerStrategy == "LRU"){
		bufmanager = new bufManagerByLRU(this,blockSize,totalBufSize);
	}else if(bufManagerStrategy == "LFU"){
		bufmanager = new bufManagerByLFU(this,blockSize,totalBufSize);
	}else if(bufManagerStrategy =="DW"){
		bufmanager = new bufManagerByDW(this,blockSize,totalBufSize,period);
	}else if(bufManagerStrategy == "LFRU"){
		bufmanager = new bufManagerByLFRU(this,blockSize,totalBufSize,period);
	}else if(bufManagerStrategy == "LRFU"){
		bufmanager = new bufManagerByLRFU(this,blockSize,totalBufSize,clientConfigInfo->lrfuLambda);
	}
}

Client::~Client(){
	pthread_mutex_destroy(&client_mutex);
}

int
Client::setRequestFile(unsigned int fileId){
	requestFileId = fileId;
	return 0;
}
int
Client::setConfigFile(const string& file){
	configFile = file;
	read_clientcircle_config(configFile);
	return 0;
}

int
Client::setMaxLoad(unsigned int load){
	maxLoad = load;
	return 0;
}


int
Client::getClientId() const{
	return clientId;
}

int 
Client::getCurrentLoad() {
	int result ;
	pthread_mutex_lock(&client_mutex);
	result = currentLoad;
	pthread_mutex_unlock(&client_mutex);
	return result;
}
int
Client::increaseCurrentLoad(){
	pthread_mutex_lock(&client_mutex);
	currentLoad ++;
	pthread_mutex_unlock(&client_mutex);
	return 0;
}
int
Client::decreaseCurrentLoad(){
	pthread_mutex_lock(&client_mutex);
	currentLoad --;
	pthread_mutex_unlock(&client_mutex);
	return 0;
}
unsigned int
Client::getBlockFromBuf(){
	unsigned int result ;
	pthread_mutex_lock(&client_mutex);
	result = blockFromBuf;
	pthread_mutex_unlock(&client_mutex);
	return result;
}
unsigned int
Client::getBlockFromClients(){
	unsigned int result ;
	pthread_mutex_lock(&client_mutex);
	result = blockFromClients;
	pthread_mutex_unlock(&client_mutex);
	return result;
	return blockFromClients;
}
unsigned int 
Client::getBlockFromServer(){
	unsigned int result ;
	pthread_mutex_lock(&client_mutex);
	result = blockFromServer;
	pthread_mutex_unlock(&client_mutex);
	return result;
}


int 
Client::increaseBlockFromBuf(){
	pthread_mutex_lock(&client_mutex);
	blockFromBuf++;
	requestSegmentNum ++;
	pthread_mutex_unlock(&client_mutex);
	return 0;
}

int
Client::increaseBlockFromServer(){
	pthread_mutex_lock(&client_mutex);
	blockFromServer++;
	requestSegmentNum ++;
	pthread_mutex_unlock(&client_mutex);
	return 0;
}

int
Client::increaseBlockFromClient(){
	pthread_mutex_lock(&client_mutex);
	blockFromClients++;
	requestSegmentNum ++;
	pthread_mutex_unlock(&client_mutex);
	return 0;
}

int 
Client::decreaseAvailableBand(double bitrate){
	int retcode;
	pthread_mutex_lock(&client_mutex);
	if(availableBand > bitrate){
		availableBand -= bitrate;
		retcode = 0;
	}else
		retcode = -1;
	pthread_mutex_unlock(&client_mutex);
	return retcode;
}

int
Client::increaseAvailableBand(double band){
	pthread_mutex_lock(&client_mutex);
	availableBand += band;
	if(availableBand > bandwidth){
		cout<<"program error in increase bandwidth";
		exit(1);
	}
	pthread_mutex_unlock(&client_mutex);
	return 0;
}

unsigned int
Client::getAvailableBand(){
	unsigned int result ;
	pthread_mutex_lock(&client_mutex);
	result = availableBand;
	pthread_mutex_unlock(&client_mutex);
	return result;
	
}
int
Client::addAlarmEvent(TimeCallBack *callBack){
	callBack->clockInfo = clientId;
	clientMaster->addAlarmEvent(callBack);
	return 0;
}
int 
Client::setClientStatus(unsigned int currentStatus){
	pthread_mutex_lock(&client_mutex);
	clientStatus = currentStatus;
	pthread_mutex_unlock(&client_mutex);
	return 0;
}
int 
Client::sendExitMessage(int sockfd,unsigned int clientId){
	ClientState exitMessage;
	exitMessage.messageType = htonl(CLIENTQUIT);
	exitMessage.clientId = htonl(clientId);
	exitMessage.availableBand = htonl(getAvailableBand());
	setClientStatus(STOP);
	if(writen(sockfd,(void*)&exitMessage,sizeof(ClientState)) !=sizeof(ClientState)){
		perror("write exit message error");
		exit(1);
	}
	close(sockfd);
	return 0;
}

int
Client::sendCurrentLoad(){
	ClientState loadMessage;
	loadMessage.messageType = htonl(HEARTBEAT);
	loadMessage.clientId = htonl(clientId);
	loadMessage.availableBand = htonl(getAvailableBand());
	//setClientStatus(STOP);
	if(writen(managerSockfd,(void*)&loadMessage,sizeof(ClientState))!=sizeof(ClientState)){
		perror("write clientState message error");
		exit(1);
	}
    return 0;

}

int
Client::sendAddedBlock(const unsigned int fileId,const unsigned int segId){
	//cout<<this->serverIp<<" "<<this->serverPort<<endl;
	BufUpdatePack packet;
	packet.messageType = htonl(BLOCKADD);
	packet.clientId = htonl(clientId);
	packet.fileId = htonl(fileId);
	packet.segId = htonl(segId);
	packet.blockSize = htonl(blockSize);
	cout<<"client "<<clientId<<" add the block <"<<fileId<<","<<segId<<">"<<endl;
	if(writen(managerSockfd,(void*)&packet,sizeof(packet))!= sizeof(packet)){
		perror("write BLOCKADD message error");
		exit(1);
	}
	return 0;
}
int
Client::sendEliminateBlock(const unsigned int fileId,const unsigned int segId){
	BufUpdatePack packet;
	packet.messageType = htonl(BLOCKELIMINATE);
	packet.clientId = htonl(clientId);
	packet.fileId = htonl(fileId);
	packet.segId = htonl(segId);
	packet.blockSize = htonl(blockSize);
	cout<<"client "<<clientId<<"eliminate the block <"<<fileId<<","<<segId<<">"<<endl;
	if(writen(managerSockfd,(void*)&packet,sizeof(packet))!= sizeof(packet)){
		perror("write BLOCKADD message error");
		exit(1);
	}
	return 0;
}

int 
Client::Run(){
	int sockfd;
	int len;
	char buf[BUFSIZE];
	VideoInfoReq rePacket;
	cout<<endl;
	cout<<"the client["<<clientId<<"] connect to the server..."<<endl;
//	requestFileId = 30;
	//requestFileId = clientId;
	rePacket.messageType = htonl(REQUESTRESOURCE);
	rePacket.clientId = htonl(clientId);
	rePacket.resourceId = htonl(requestFileId);
	rePacket.bandwidth = htonl(bandwidth);
	//send the requestsource message to server
//	cout<<"size of struct repacket "<<sizeof(rePacket)<<endl;

	while(true){
		cout<<"create connection socket"<<endl;
		sockfd = tcp_connect(serverIp.c_str(),serverPort.c_str());
		if(sockfd < 0){
			cout<<"tcp connect error"<<endl;
			continue;
		}
		if(writen(sockfd,(void*)&rePacket,sizeof(rePacket))!= sizeof(rePacket)){
			cout<<"write error: client send the REQUESTSOURCE message"<<endl;
			sendExitMessage(sockfd,clientId);
			exit(1);
			continue;
		}else{
		//	cout<<"write ok"<<endl;
		}
	//recv the requestsource ack message
		if((len = readn(sockfd,(void*)buf,sizeof( VideoInfo)))!= sizeof( VideoInfo)){
			perror("recv error");
			cout<<"read error:client "<<clientId<<" failed to recv REQUESTSOURCEACK message"<<endl;
			cout<<"read len = "<<len<<endl;
			exit(1);
			if(len <=0){
				close(sockfd);
				cout<<"client "<<clientId<<" exit  in "<<__FUNCTION__<<endl;
				pthread_exit(NULL);
			}else{
				sendExitMessage(sockfd,clientId);
			}
			continue;
		}
		break;
	}
	VideoInfo *video_info = (VideoInfo*)buf;
	requestFileLen = ntohl(video_info->fileLength);
	requestFileBitRate = video_info->bitRate;
	cout<<"request fileLen = "<<requestFileLen<<endl;
	cout<<"requestFileBitRate = "<<requestFileBitRate<<endl;
	clientStatus = PLAY;
	requestSegment = 0;
	managerSockfd = tcp_connect(getServerIp().c_str(),managerPort.c_str());
//	playingClient(sockfd);
//	if(_needRecord == true)
		clientLifeCircle( sockfd);
	return 0;
}


typedef struct NextStatus{
	unsigned int nextStatus;
	unsigned int prob;
}NextStatus;

bool compare(const struct NextStatus &ns1,const struct NextStatus &ns2){
	if(ns1.prob > ns2.prob)
		return false;
	else return true;
}

int 
Client::getNextStatus(unsigned int& currentStatus){
	if(currentStatus == PAUSE || currentStatus == FORWARD || currentStatus == BACKWARD){
		currentStatus = PLAY;
	}else if(currentStatus == PLAY){
		vector<NextStatus> ns;
		//for play
		NextStatus play ;
		play.nextStatus = PAUSE;
		play.prob = playToPlay;
		ns.push_back(play);
		
		//for pause
		NextStatus pause ;
		pause.nextStatus = PAUSE;
		pause.prob = playToPause;
		ns.push_back(pause);
		//for stop
		NextStatus stop;
		stop.nextStatus = STOP;
		stop.prob = playToStop;
		ns.push_back(stop);
		//for forward
		NextStatus forward;
		forward.nextStatus = FORWARD;
		forward.prob = playToForward;
		ns.push_back(forward);
		//for backward
		NextStatus backward;
		backward.nextStatus = BACKWARD;
		backward.prob = playToBackward;
		ns.push_back(backward);
		sort(ns.begin(),ns.end(),compare);
		int temp = Randomi(0,1000);
		int sum = 0;
		for(size_t i=0;i<ns.size();i++){
			sum+=ns[i].prob;
			if(sum>=temp){
				currentStatus = ns[i].nextStatus;
				break;
			}
		}
	}
	return 0;
}


int
Client::playingClient(int sockfd){
	unsigned int fileId,segId;
	string action;
	int j= bufRecordFile.find('.');
	string bufRecord=(bufRecordFile).substr(0,j) + numToString(clientId) + bufRecordFile.substr(j);
	ifstream ifs(bufRecord.c_str());
	if(ifs){
		cout<<bufRecord <<" existed..."<<endl;
		while(!ifs.eof()){
			ifs >> fileId >> segId >> action;
			cout<<"for client "<<clientId<<" : "<< "will request < "<<fileId<<","<<segId<<">"<<endl;
			if(action.compare("Read") == 0 || action.compare("ADD")==0){
				requestFileId = fileId;
				requestSegment = segId;
				playSegment(sockfd,blockSize);
			}
		}
	}else{
		_needRecord = true;
	}
	return 0;
}

int 
Client::clientLifeCircle(int sockfd){
	unsigned int segs = (requestFileLen%blockSize ==0)?(requestFileLen/blockSize):(requestFileLen/blockSize + 1);
	cout<<"client "<<clientId<<" request file "<<requestFileId<<",which have "<<segs<<"segs"<<endl;
	cout<<"request file length = "<<requestFileLen<<endl;
	cout<<"blockSize = "<<blockSize<<endl;
/*	ifstream ifs;
	ofstream ofs;
	if(clientId ==0){
		cout<<"try to open requestSegment.txt"<<endl;
		ifs.open("data/requestSegment.txt");
		unsigned int fileId,segId;
		if(ifs.is_open()){
			cout<<"open requestSegment.txt ok!"<<endl;
			while(ifs >> fileId >> segId){
				playSegment(sockfd,fileId,segId);
			}
		}else{
			ofs.open("data/requestSegment.txt");
			if(!ofs.is_open()){
				exit(1);
			}
		}
	}	*/
	while(clientStatus != STOP){
		unsigned int length = rand_lognormal(myzeta,mysigma); //in seconds 
//		cout<<"jumpLength = "<<length<<endl;
		unsigned int jumpSegs;
		unsigned int jumpLength = length*requestFileBitRate/8;
		cout<< "jumpLength = "<<jumpLength<<endl;
		switch(clientStatus){
			case PLAY:
				if(jumpLength + requestSegment*blockSize > requestFileLen){
					jumpLength = requestFileLen - requestSegment  *blockSize;
				}
				playSegment(sockfd,jumpLength );
			
				//requestSegment +=jumpSeg;
				getNextStatus(clientStatus);
				break;
			case PAUSE:
				usleep((length * 8000000)/requestFileBitRate);
				getNextStatus(clientStatus);
				break;
			case FORWARD:
				//sleep(JUMPMINUTES);
				if(jumpLength + requestSegment*blockSize > requestFileLen){
					jumpLength = requestFileLen - requestSegment  *blockSize;
				}
				requestSegment += jumpLength/blockSize ;
				if(requestSegment > segs){
					cout<<"client "<<clientId<<" in play,request Segment = "<<requestSegment<<" error"<<endl;
					cout<<"jumpLen= "<<jumpLength<<endl;
					requestSegment =0;
	//				exit(1);
				}
			//	playSegment(sockfd,requestFileId,requestSegment);
				//requestFileSegments(sockfd,requestFileId,requestSegment,prefetchNumber);
				getNextStatus(clientStatus);
				break;
			case BACKWARD:
			//pp	getNextSegment(clientStatus);
			//	sleep(JUMPMINUTES);
				jumpSegs = jumpLength / blockSize;
				if(requestSegment*blockSize <= jumpLength )
					requestSegment = 0;
				else
					requestSegment -=jumpSegs;
				if(requestSegment < 0){
					cout<<"client "<<clientId<<" in jump backward ,request Segment = "<<requestSegment<<" error"<<endl;
					cout<<"jumpSegs = "<<jumpSegs<<endl;
					requestSegment = 0;
					exit(1);
				}
			//	playSegment(sockfd,requestFileId,requestSegment);
				//requestFileSegments(sockfd,requestFileId,requestSegment,prefetchNumber);
				getNextStatus(clientStatus);
				break;
			case STOP:
				cout<<"the client :"<<clientId<<"exited"<<endl;
				//should do something 
				//sendStopMessage(clientId);
				sendExitMessage(sockfd,clientId);
				break;
		}
	}
	return 0;
}

int 
Client::playSegment(int sockfd,unsigned int fileLen){
	while(fileLen > 0){
		if(true == bufmanager->haveSegment(requestFileId,requestSegment)){
			increaseBlockFromBuf();
			unsigned int sleepTime =blockSize*8000000/requestFileBitRate;
		//	cout<<"playing "<<sleepTime<<" microseconds"<<endl;
			bufmanager->readFileSegment(requestFileId,requestSegment);
			bufmanager->readBlock(requestFileId,requestSegment);
			mysleep(sleepTime);
			requestSegment ++;
			fileLen -= blockSize;
		}else{
			if(requestFileSegments(sockfd,requestFileId,requestSegment)==-1){
				cout<<"client "<<clientId<<" exit  in "<<__FUNCTION__<<endl;
				exit(1);
	            //pthread_exit(NULL);
		    }else{
				requestSegment ++;
				fileLen -= blockSize;
				
			}
		}

	}
	return 0;
}
int 
Client::requestFileSegments(int sockfd,unsigned int fileId,unsigned int segId){
	VodSegReq segments;
	char buf[BUFSIZE];
	segments.messageType = htonl(REQUESTSEGMENT);
	segments.resourceId = htonl(fileId);
	segments.clientId = htonl(clientId);
	segments.sidStart = htonl(segId);
	segments.sidNum = htonl(1);
	cout<<"client "<<clientId<<" request <"<<fileId<<","<<segId<<">"<<endl;
	if(writen(sockfd,(void*)&segments,sizeof( VodSegReq))<0){
		perror("in client thread,send REQUESTSEGMENT message error");
		exit(1);
	}
	if(sizeof(vodSeg) !=readn(sockfd,(void*)buf,sizeof(VodSeg))){
		perror("read vod seg ack message error");
		exit(1);
	}
	struct vodSeg *segPtr = (struct vodSeg*)buf;	
	cout<<"clientId = "<<clientId<<",server reply = "<<ntohl(segPtr->clientId)<<endl;
	if(ntohl(segPtr->clientId) == SERVERID){
		cout<<"client "<<clientId<<" get the segment <"<<ntohl(segPtr->resourceId)<<","<<ntohl(segPtr->sid)<<"> from server"<<endl;
		clientMaster->increaseServerCounts();
		if(decreaseAvailableBand(requestFileBitRate)<0){
			cout<<"no available bandwidth"<<endl;
			exit(1);
		}
		bufmanager->writeFileSegment(requestFileId,requestSegment);
		unsigned int sleepTime =blockSize*8000000/requestFileBitRate;
		cout<<"playing "<<sleepTime<<" microseconds"<<endl;
		mysleep(sleepTime);
		increaseBlockFromServer();
		increaseAvailableBand(requestFileBitRate);
	}else{
		cout<<"client "<<clientId<<" will get the segment <"<<ntohl(segPtr->resourceId)<<","<<ntohl(segPtr->sid);
		cout<<"> from client "<<ntohl(segPtr->clientId)<<endl;
		int result;
		if(decreaseAvailableBand(requestFileBitRate)<0){
			cout<<"no available bandwidth"<<endl;
			exit(1);
		}
		result = requestFileSegment(ntohl(segPtr->clientId),ntohl(segPtr->resourceId),requestSegment);
		increaseAvailableBand(requestFileBitRate);
		if(result < 0){		//can't find this segment in target client 
			cout<<"request File segment error "<<endl;
		//	requestFileSegments(sockfd,fileId,segId);	
			segments.messageType = htonl(REQUESTSEGMENTAGAIN);
			segments.resourceId = htonl(fileId);
			segments.clientId = htonl(clientId);
			segments.sidStart = htonl(segId);
			segments.sidNum = htonl(1);
			cout<<"client "<<clientId<<" request <"<<fileId<<","<<segId<<"> again"<<endl;
			if(writen(sockfd,(void*)&segments,sizeof( VodSegReq))!= sizeof( VodSegReq)){
				perror("in client thread,send REQUESTSEGMENT message error");
				exit(1);
			}
			if(sizeof(vodSeg) !=readn(sockfd,(void*)buf,sizeof(VodSeg))){
				perror("read vod seg ack message error");
				exit(1);
			}
			segPtr= (vodSeg*)buf;
			if(ntohl(segPtr->clientId) ==SERVERID){
				cout<<"client "<<clientId<<" get the segment <"<<ntohl(segPtr->resourceId)<<","<<ntohl(segPtr->sid)<<"> from server"<<endl;
				clientMaster->increaseServerCounts();
				if(decreaseAvailableBand(requestFileBitRate)<0){
					cout<<"no available bandwidth"<<endl;
					exit(1);
				}
				bufmanager->writeFileSegment(requestFileId,requestSegment);
				int sleepTime =blockSize*8000000/requestFileBitRate;
				cout<<"playing "<<sleepTime<<" microseconds"<<endl;
				mysleep(sleepTime);
				increaseBlockFromServer();
				increaseAvailableBand(requestFileBitRate);
			}else{
					increaseAvailableBand(requestFileBitRate);
					cout<<"should get the resource from server!"<<endl;
					exit(1);	
			}
			//	exit(1);
		}else {
			clientMaster->increaseTotalCounts();
			bufmanager->writeFileSegment(ntohl(segPtr->resourceId),requestSegment);
			increaseBlockFromClient();
		
		}
	}
	/*
	for(size_t i = 0;i<messageLen / sizeof(struct vodSeg);i++){
		segPtr = (vodSeg*)(buf + i*sizeof(struct vodSeg));
		if(ntohl(segPtr->clientId) ==SERVERID){
			cout<<"client "<<clientId<<" get the segment <"<<ntohl(segPtr->resourceId)<<","<<ntohl(segPtr->sid)<<"> from server"<<endl;
			clientMaster->increaseServerCounts();
			bufmanager->writeFileSegment(ntohl(segPtr->resourceId),ntohl(segPtr->sid));
		}else{
			cout<<"client "<<clientId<<" will get the segment <"<<ntohl(segPtr->resourceId)<<","<<ntohl(segPtr->sid);
			cout<<"> from client "<<ntohl(segPtr->clientId)<<endl;
			if(requestFileSegment(ntohl(segPtr->clientId),ntohl(segPtr->resourceId),ntohl(segPtr->sid))<0){
			
				segments.messageType = htonl(REQUESTSEGMENTAGAIN);
				segments.resourceId = htonl(fileId);
				segments.clientId = htonl(clientId);
				segments.sidStart = htonl(sidStart + i);
				segments.sidNum = htonl(1);
				cout<<"client "<<clientId<<" request <"<<fileId<<","<<sidStart + i<<">"<<endl;
				if(writen(sockfd,(void*)&segments,sizeof( VodSegReq))!= sizeof( VodSegReq)){
					perror("in client thread,send REQUESTSEGMENT message error");
					return -1;
				}
				unsigned int messageLen = sidNum * sizeof(struct vodSeg);
				messageLen = readn(sockfd,(void*)buf,messageLen);
				if(messageLen != sizeof(struct vodSeg) * sidNum ){
					perror("in client thread,recv REQUESTSEGMENT ack message error");
					return -2;
				}
				struct vodSeg *segPtr;
				for(size_t i = 0;i<messageLen / sizeof(struct vodSeg);i++){
					segPtr = (vodSeg*)(buf + i*sizeof(struct vodSeg));
					if(ntohl(segPtr->clientId) ==SERVERID){
						cout<<"client "<<clientId<<" get the segment <"<<ntohl(segPtr->resourceId)<<","<<ntohl(segPtr->sid)<<"> from server"<<endl;
						clientMaster->increaseTotalCounts();
						clientMaster->increaseServerCounts();
						bufmanager->writeFileSegment(ntohl(segPtr->resourceId),ntohl(segPtr->sid));
					}else{
						cout<<"should get the resource from server!"<<endl;
						return -1;
					}
				}
			}
	
		}
		clientMaster->increaseTotalCounts();
		int sleepTime =blockSize*8000000/requestFileBitRate;
		cout<<"playing "<<sleepTime<<" microseconds"<<endl;
	//	readFileSegment(fileId,sidStart);
		usleep(sleepTime);
	}*/
	return 0;
}

int 
Client::requestFileSegment(unsigned int clientId,const unsigned int fileId,unsigned int segId){
	if(clientMaster->requestSegment(clientId,fileId,segId,requestFileBitRate)<0){
		cout<<"can't find the segment <"<<fileId<<","<<segId<<">"<<" in "<<clientId<<endl;
		return -1;
	}else{
		bufmanager->writeFileSegment(fileId,segId);
		return 0;
	}
}



int
Client::sendTimeTicks(double band){
    sendCurrentLoad();
    unsigned int sleepTime =   blockSize * 8000000/band;
    mysleep(sleepTime);
    sendCurrentLoad();
    return 0;
}
int
Client::receiveRequest(unsigned int fileId,unsigned int segId,double bitrate){
	int retcode =0;
	if(bufmanager->readFileSegment(fileId,segId)<0){
		retcode = -1;
	}else if(decreaseAvailableBand(bitrate)< 0){
		cout<<"un available bandwidth"<<endl;
		retcode = -2;
		exit(1);
	}else{
		sendTimeTicks(bitrate);
		increaseAvailableBand(bitrate);
		retcode = 0;
	}
    return retcode;
}
int
Client::read_clientcircle_config(const string &fileName){
    ifstream confFile( fileName.c_str() );

    if ( !confFile.is_open() ) {
        cout << "\tFatal Error : Cannot open configure file "
             << fileName << endl;
        exit(-1);
    }
	//cout<<"for client["<<clientId<<"],read client circle configure file"<<endl;
    string line;
    while( confFile.good() ) {
        getline( confFile, line );
		string key,value;
        parse_config_line( line ,key,value);
		if(key.size()==0 || value.size()==0)
			continue;
		//cout<<"key = "<<key<<" value = "<<value<<endl;
		if(key.compare("zeta") ==0){
			myzeta = stringToInt(value)/1000.0;
		//	cout<<"myzeta = "<<myzeta<<endl;
		}
		else if(key.compare("sigma")==0){
			mysigma = stringToInt(value)/1000.0;
		//	cout<<"mysigma = "<<mysigma<<endl;
		}
		else if(key.compare("playToPlay") ==0){
			playToPlay = atoi(value.c_str());
		}else if(key.compare("playToPause") ==0){
			playToPause = atoi(value.c_str());
		}else if(key.compare("playToStop") == 0){
			playToStop = atoi(value.c_str());
		}else if(key.compare("playToForward")==0){
	
			playToForward = atoi(value.c_str());
		}else if(key.compare("playToBackward")==0){
			playToBackward = atoi(value.c_str());
		}else if(key.compare("pauseToPlay") == 0){
			pauseToPlay = atoi(value.c_str()); 
		}else if(key.compare("forwardToPlay")==0){
			forwardToPlay = atoi(value.c_str());
		}else if(key.compare("backwardToPlay")==0){
			backwardToPlay = atoi(value.c_str());
		}else{
			cout<<"can't find the args:"<<key<<endl;
			continue;
		}
	}
	confFile.close();
	return 0;
}
