#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H


#include "../include/message.h"
#include "../include/global.h"
#include "client.h"
#include "config.h"
/*
 * used for send and receive vod resources from server
 */
void* clientRequestThread(void *arg);
void* checkAlarmEventThread(void *arg);
/*
 * used for send current load and buffer to the server 
 */
void* clientInfoThread(void *arg); 

class ClientManager{
	public:
		ClientManager(ClientConfigType *clientConfigInfo);
		~ClientManager();
	public:
		int getTimeInterval(double lambda,int n,vector<int> &timeInterval);
		int createRequestList(vector<unsigned int> &fileList);
		int createZipfDistribution(double sita,vector<double> &probability);
		int createClientList();
		int addAlarmEvent(TimeCallBack *alarmEvent);
		int removeAlarmEvent(TimeCallBack *alarmEvent);
		int checkAlarmEvent();
		int Run();
		int destroyClientList();


	public:
		int increaseTotalCounts();
		int getTotalCounts();
		int increaseServerCounts();
		int getServerCounts();
		int doStatistical();

	public:
		int setRequestFile(const vector<unsigned int>& fileList,int n);
		int setMaxLoad(int maxLoad,int i);
		int setCircleConfigFile(unsigned int clientId,const string& configFile);
		int requestSegment(unsigned int clientID,unsigned int fileId,unsigned int segId,double band);
	
	private:
		ClientConfigType *clientConfig;
		vector<Client*> clientList;
		pthread_mutex_t time_mutex;
		vector<TimeCallBack *> scheduleList;
		bool needTimeScheduleList;
	private:
		pthread_mutex_t count_mutex;
		unsigned int requestTotals;
		unsigned int requestServerTimes;
};



#endif
