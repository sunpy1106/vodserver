#ifndef POOL_H
#define POOL_H

#include <deque>
#include <vector>
#include <string>
#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cstring>


#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

extern "C"{
#include <pthread.h>
#include <assert.h>
#include <linux/types.h>
}


#include "../include/global.h"
#include "config.h"
using namespace std;

class MYSQLPool{
    public:
        MYSQLPool(const string filename);
        ~MYSQLPool();

        sql::Connection* getConnection();
        void releaseConnection(sql::Connection*);
		int getCanUsedConnections();
		int getPooledConnections();
	public:       
		int deleteAllResources();
		int insertResource(unsigned int fileId,unsigned int fileSize,float bitRate);
		int getFileInfo(unsigned int fileId,unsigned int &fileLen,float &bitRate);
		int insertP2pClient(unsigned int id,unsigned int maxBand,unsigned int currentBand);
		int updateP2pClient(unsigned int clientId,unsigned int currentId);
		int insertFileSegment(unsigned int fileId,unsigned int segId,unsigned int clientId,unsigned int size);
		int deleteFileSegment(unsigned int fileId,unsigned int segId,unsigned int clientId);
		int getFileSegInfo(unsigned int fileId,unsigned int segId,unsigned int &clientId);
		int deleteClient(unsigned int clientId);
		int resetClient();
		int resetClientSegment();
		int deleteClientBuf(unsigned int clientId);
		int getAvailableBand();
		int getClientBand(unsigned int id);
    private:
        sql::Connection* createConnection();
        void freeConnection(sql::Connection*);
        void pingConnection(sql::Connection*);

        static  void* thrfun(void*);
        
        void keepPoolAlive();

        bool isValid;

        deque<sql::Connection*> pool_deque;
        pthread_mutex_t mutex;
        pthread_cond_t cond;

        pthread_t tid;

        string host;
        string user;
        string passwd;
        string database;

        int keepAliveTimeout;
        int minConnections;
        int maxConnections;
        int pooledConnections;

        sql::Driver* driver;
};

#endif
