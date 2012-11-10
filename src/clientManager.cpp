#include "clientManager.h"


void * clientRequestThread(void *arg){
	Client *curClient = (Client*)arg;
	curClient->Run();
	return NULL;
}

void *checkAlarmEventThread(void *arg){
	ClientManager *clientMaster = (ClientManager*)arg;
	clientMaster->checkAlarmEvent();
	return NULL;
}

void *statisticalThread(void *arg){
	ClientManager *clientMaster = (ClientManager*)arg;
	clientMaster->doStatistical();
	return NULL;
	
}

ClientManager::ClientManager(ClientConfigType *clientConfigInfo){
	clientConfig = clientConfigInfo;
	requestTotals = requestServerTimes = 0;

	pthread_mutex_init(&count_mutex,NULL);
	needTimeScheduleList =false;
	pthread_mutex_init(&time_mutex,NULL);

}

ClientManager::~ClientManager(){
	destroyClientList();
	pthread_mutex_destroy(&time_mutex);
}

int
ClientManager::addAlarmEvent(TimeCallBack* alarmEvent){
	if(needTimeScheduleList == false){
		needTimeScheduleList = true;
		pthread_t tid1;
		pthread_create(&tid1,NULL,&checkAlarmEventThread,this);
		pthread_detach(tid1);
	}
	pthread_mutex_lock(&time_mutex);
	scheduleList.push_back(alarmEvent);
	pthread_mutex_unlock(&time_mutex);
	return 0;
}

int
ClientManager::increaseTotalCounts(){
	pthread_mutex_lock(&count_mutex);
	requestTotals ++;
	pthread_mutex_unlock(&count_mutex);
	return 0;
}

int 
ClientManager::getTotalCounts(){
	unsigned int result;
	pthread_mutex_lock(&count_mutex);
	result = requestTotals ;
	pthread_mutex_unlock(&count_mutex);
	return result;
}

int
ClientManager::getServerCounts(){
	unsigned int result;
	pthread_mutex_lock(&count_mutex);
	result = requestServerTimes;
	pthread_mutex_unlock(&count_mutex);
	return result;
}

int
ClientManager::increaseServerCounts(){
	pthread_mutex_lock(&count_mutex);
	requestServerTimes ++;
	requestTotals ++;
	pthread_mutex_unlock(&count_mutex);
	return 0;
}


int
ClientManager::doStatistical(){
	ofstream ofs;
	cout<<"*********************open the static file ************************************"<<endl;
	ofs.open("data/clientResult.txt");
	ofs<<"totalHitRate\t"<<endl;
	unsigned long nowTotal,nowClients;
	unsigned int blockfromBufs,blockfromServer,blockfromClients,totalNum;
	unsigned int a,b;
	unsigned int maxClientId;
	double maxHitRate,hitRate ;
	for(unsigned int k= 0;;k++){
			ofs<<"**********************test sample "<<k<<"*******************"<<endl;
			sleep(2);
			nowTotal  = getTotalCounts();
			nowClients = nowTotal -  getServerCounts();
		//	ofs<<nowTotal<<"\t\t\t"<<nowTotal-nowClients<<"\t\t";
			if(nowTotal ==0)
				ofs<<"0"<<endl;
			else
				ofs<<nowClients/(double)nowTotal<<endl;	
			maxHitRate = 0.0;
			a = b = 0;
			for(unsigned int i = 0;i<clientList.size();i++){
				blockfromBufs = clientList[i]->getBlockFromBuf();
				blockfromServer = clientList[i]->getBlockFromServer();
				blockfromClients = clientList[i]->getBlockFromClients();
				totalNum = blockfromBufs + blockfromServer + blockfromClients;
				if(clientList[i]->getRequestFileId() <=1){
					a += blockfromServer;
					b += totalNum;
				}
				if(totalNum !=0){
					hitRate = (totalNum - blockfromServer)/(double) totalNum;
					ofs<<"client "<<i<<":"<<blockfromBufs<<"\t"<<blockfromServer<<"\t"<<blockfromClients<<"\t";
					ofs<<totalNum<<"\t"<<hitRate<<endl;
				}
				if(hitRate > maxHitRate){
					maxHitRate = hitRate;
					maxClientId = i;
				}
			}
			ofs<<"the client "<<maxClientId<<" have the max hitrate = "<<maxHitRate<<endl;
			ofs<<"the hitrate of file 0 is "<<(b-a)/(double) b<<endl;
			ofs.flush();
	}
	ofs.close();
	return 0;
}

int
ClientManager::removeAlarmEvent( TimeCallBack* alarmEvent){
	pthread_mutex_lock(&time_mutex);
	vector<TimeCallBack*> ::iterator it;
	for(it = scheduleList.begin();it!=scheduleList.end();it++){
		if(*it == alarmEvent){
			scheduleList.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&time_mutex);
	return 0;
}

int 
ClientManager::checkAlarmEvent(){
	while(true){
		pthread_mutex_lock(&time_mutex);
		vector<TimeCallBack*> ::iterator it;
		for(it = scheduleList.begin();it!=scheduleList.end();it++){
			(*it)->timeleft -=1;
			if((*it)->timeleft == 0){
				((*it)->callBackFunc)((*it)->arg);
				(*it)->timeleft = (*it)->period ;
			}
		}
		pthread_mutex_unlock(&time_mutex);
		mysleep(1000000);
	}
	return 0;
}

int 
ClientManager::createZipfDistribution(double skew,vector<double> & probability){
	int n = clientConfig->resourceNumber;
	double denominator = 0.0;
	double u;
	for(int i=0;i<n;i++){				// get the sum of pow(i+1,-skew);
		denominator += pow(i+1,0-skew);
	}
	for(int i=0;i<n;i++){				// get the u1,u2,u3,...
		u = pow(i+1,0-skew);
		cout<<"u = "<<u<<endl;
		u = u / denominator;
		probability.push_back(u);
		cout<<"probability of file "<<i<<" is "<<u<<endl;
	}
	return 0;
}
int
ClientManager::Run(){
	createClientList();
	while(true){
		pause();	
	}
	return 0;	
}
int 
ClientManager::createRequestList(vector<unsigned int> &fileList){
	double sita = clientConfig->zipfParameter;
	int n  = clientConfig->clientNumber;
    int m = clientConfig->resourceNumber;
	vector<double> probability;
	createZipfDistribution(sita,probability);
	fileList.clear();
	for(int i =0;i<n;i++){
		double temp = Randomf(0,1);
		double sum = 0.0;
		for(int i =m-1;i>=0 ;i--){
			sum +=probability.at(i);
			if(sum > temp ){
				fileList.push_back(i);
				break;
			}
		}
	}
	return 0;
}


int 
ClientManager::createClientList(){
	int n = clientConfig->clientNumber;
	vector<unsigned int> fileList(n);
	vector<int> timeInterval(n);
	int j = (clientConfig->requestListFile).find('.');
	string requestListFile = (clientConfig->requestListFile).substr(0,j)+numToString(n) + (clientConfig->requestListFile).substr(j);
	ifstream ifs(requestListFile.c_str());
	if(!ifs){
		//get the sequence of video requests;
		createRequestList(fileList);
		//get the request time interval
		getTimeInterval(clientConfig->poissonLambda,n,timeInterval);
		ofstream ofs(requestListFile.c_str());
		for(int i =0;i<n;i++){
			ofs << fileList.at(i)<<" "<<timeInterval.at(i)<<endl;
		}
		ofs.close();
	}else{
		fileList.clear();
		timeInterval.clear();
		unsigned int a,b;
		while(!ifs.eof()){
			ifs >>a >>b;
			fileList.push_back(a);
			timeInterval.push_back(b);
		}
		ifs.close();
	}
	for(int i =0;i<n;i++){
		cout<<"client "<<i<<" request file "<<fileList.at(i)<<" after "<<timeInterval.at(i)<<" time intervals"<<endl;
	}
	for(int i =0;i<n;i++){
		Client *clienti = new Client(i,clientConfig,this);
		clientList.push_back(clienti);
	}
	setRequestFile(fileList,n);
	pthread_t tid;
	unsigned int configFileIndex;
	pthread_t tid0;
	pthread_create(&tid0,NULL,&statisticalThread,this);
	pthread_detach(tid0);
	for(int i = 0;i<n;i++){
		//get the maxLoad of current client

		configFileIndex = Randomi(1,clientConfig->clientTypes);
		string configFile = (clientConfig->circleConfigFile);
		size_t j,len = configFile.length();
		j = configFile.find('.',0);
		string myConfigFile = configFile.substr(0,j) + numToString(configFileIndex) + configFile.substr(j,len-1);
		setCircleConfigFile(i,myConfigFile);
		//create the request thread
		usleep(timeInterval.at(i));
		pthread_create(&tid,NULL,&clientRequestThread,(void*)clientList[i]);
	}
	
	//clientRequestThread((void*)clientList[0]);
	return 0;
}


int
ClientManager::getTimeInterval(double lambda,int n,vector<int> &timeInterval){
	double temp;
	timeInterval[0] =0;
	for(int i = 1;i<n;i++){
		temp = Randomf(0,1);
		temp  = 1 - temp;
		temp = 0 - log(temp)/lambda;
		timeInterval[i]  = (int)(temp * 1000000);
	}
	return 0;
}

int
ClientManager::setRequestFile(const vector<unsigned int> &fileList,int n){
	for(int i = 0;i<n;i++){
		clientList[i]->setRequestFile(fileList.at(i));
	}
	return 0;
}


int 
ClientManager::setCircleConfigFile(unsigned int i,const string& configFile){
	clientList[i]->setConfigFile(configFile);
	return 0;
}

int 
ClientManager::setMaxLoad(int maxLoad,int i){
	clientList[i]->setMaxLoad(maxLoad);
	return 0;
}


int 
ClientManager::destroyClientList(){
	for(size_t i= 0;i<clientList.size();i++)
		delete clientList[i];
	clientConfig = NULL;
	return 0;
}

/*
 *help one client to request the segment of other client
 */
int 
ClientManager::requestSegment(unsigned int clientId,unsigned int fileId,unsigned int segId,double bitrate){
	if(clientId >= clientList.size()){
		cout<<"wrong clientId:"<<clientId<<endl;
		return -1;
	}
    return 	(clientList[clientId])->receiveRequest(fileId,segId,bitrate);
}
