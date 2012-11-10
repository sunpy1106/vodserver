#include "MYSQLPool.h"

#define MAXLINE 1024

using namespace std;

void* 
MYSQLPool::thrfun(void* arg){
    ((MYSQLPool*)arg)->keepPoolAlive();
    return NULL;
}

MYSQLPool::MYSQLPool(const string filename){  
    /* 读取相关配置文件 */
    map<string,string> conf_map;     
    if(read_config_file(filename,conf_map) != 0){
        err_quit("can not open config file %s",filename.c_str());
    }
    //主机
    if(conf_map.find("host") == conf_map.end()){
        err_quit("can not find host in config file");
    }
    host = conf_map["host"].c_str();

    //用户名
    if(conf_map.find("user") == conf_map.end()){
        err_quit("can not find user in config file");
    }
    user = conf_map["user"].c_str();

    //密码
    if(conf_map.find("passwd") == conf_map.end()){
        err_quit("can not find passwd in config file");
    }
    passwd = conf_map["passwd"].c_str();

    //数据库名
    if(conf_map.find("database") == conf_map.end()){
        err_quit("can not find database in config file");
    }
    database = conf_map["database"].c_str();

    //最小连接数
    if(conf_map.find("minConnections") == conf_map.end()){
        err_quit("can not find minConnections in config file");
    }
    minConnections = atoi(conf_map["minConnections"].c_str());

    //最大连接数
    if(conf_map.find("maxConnections") == conf_map.end()){
        err_quit("can not find maxConnections in config file");
    }
    maxConnections = atoi(conf_map["maxConnections"].c_str());

    //保活时间间隔
    if(conf_map.find("keepAliveTimeout") == conf_map.end()){
        err_quit("can not find keepAliveTimeout in config file");
    }
    keepAliveTimeout = atoi(conf_map["keepAliveTimeout"].c_str());

    pooledConnections = 0;

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);

    try{
        driver = get_driver_instance();
    }catch(sql::SQLException& e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl;
    }

    int i;
    sql::Connection* con;
    for(i=0; i<minConnections; i++){
        if((con = createConnection()) != NULL){
            pool_deque.push_back(con);
        }
    }

    if(pthread_create(&tid,NULL,thrfun,(void*)this) < 0){
        err_dump("can not create thread for MYSQLPool");
    } 

    isValid = true;
}

MYSQLPool::~MYSQLPool(){
    int nleft;
    pthread_mutex_lock(&mutex); 
    isValid = false;
    pthread_cancel(tid);
    if(pthread_join(tid,NULL) < 0){
        err_dump("can not join pthread for MYSQLPool");
    }
    nleft = pooledConnections;
    pthread_mutex_unlock(&mutex); 
    while(nleft > 0){
        pthread_mutex_lock(&mutex); 
        for(unsigned i=0; i<pool_deque.size(); i++){
            freeConnection(pool_deque[i]);
        }
        nleft = pooledConnections; 
        //  cout<<nleft<<endl;
        pthread_mutex_unlock(&mutex); 
    }
    cout<<"delete MYSQLPool"<<endl;
}

sql::Connection*
MYSQLPool::createConnection(){
    sql::Connection* con;
    try{
        con = driver->connect(host,user,passwd);
        con->setSchema(database);
        pooledConnections++;
        //cout<<"create new connection. cur = "<<pooledConnections<<endl;
        return con;
    }catch(sql::SQLException& e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl;
        return NULL;  
    }
}

void
MYSQLPool::freeConnection(sql::Connection* con){
    delete con; 
    pooledConnections--;
	cout<<"releaseConnetcion:can used connections = "<<pool_deque.size()<<endl;
	cout<<"pooledConnections = "<<pooledConnections<<endl;
    //cout<<"free connection. cur = "<<pooledConnections<<endl;
}

void
MYSQLPool::pingConnection(sql::Connection* con){
    try{
        sql::Statement* stmt = con->createStatement();
        stmt->executeQuery("select 0 from dual");
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl;
    }    
}

sql::Connection* 
MYSQLPool::getConnection(){
    sql::Connection* con = NULL;
    pthread_mutex_lock(&mutex); 

    if(isValid == false){
        pthread_mutex_unlock(&mutex);
        err_dump("MYSQLPool is invalided");
    }

    while(pool_deque.size() == 0 && pooledConnections == maxConnections){
		cout<<"can used connections = "<<pool_deque.size()<<endl;
		cout<<"pooledConnections = "<<pooledConnections<<endl;
		pthread_cond_wait(&cond,&mutex);
    }
    if(pool_deque.size() > 0){
        con = pool_deque.front();
        pool_deque.pop_front();
    }else if(pooledConnections < maxConnections){
        con = createConnection();
    }

    pthread_mutex_unlock(&mutex);
    return con;
}

void
MYSQLPool::releaseConnection(sql::Connection* con){
    pthread_mutex_lock(&mutex); 
    pool_deque.push_back(con);
    pthread_cond_signal(&cond); 
    pthread_mutex_unlock(&mutex); 
}
int
MYSQLPool::getCanUsedConnections(){
    int result ;
	pthread_mutex_lock(&mutex); 
	result = pool_deque.size();
    pthread_mutex_unlock(&mutex);
	return result;
}

int
MYSQLPool::getPooledConnections(){
    int result;
	pthread_mutex_lock(&mutex); 
	result = pooledConnections;
    pthread_mutex_unlock(&mutex);
	return result;
}

void
MYSQLPool::keepPoolAlive(){
    sql::Connection* con;
    sleep(keepAliveTimeout);
    while(true){
        pthread_mutex_lock(&mutex);
        while(pool_deque.size() > (unsigned)minConnections){
            con = pool_deque.front();
            pool_deque.pop_front();
            freeConnection(con);
        }

        for(unsigned i=0;i<pool_deque.size();i++){
            pingConnection(pool_deque[i]);
        }
        pthread_mutex_unlock(&mutex); 
        sleep(keepAliveTimeout);
    }
}

int 
MYSQLPool::insertResource(unsigned int fileId,unsigned int fileSize,float bitRate){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"INSERT INTO ivod_video_t VALUES('%d','%d',NULL,'%f')",
                fileId,fileSize,bitRate);
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
    return retCode;
}
int
MYSQLPool::getFileInfo(unsigned int fileId,unsigned int &fileLen,float &bitRate){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"SELECT fileLength,bitRate FROM ivod_video_t WHERE id  = '%d'",fileId);
        res = stmt->executeQuery(sql);
        while(res->next()){
            fileLen = res->getUInt("fileLength");
			bitRate = res->getDouble("bitRate");
        }
        delete res;
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
    return retCode;
}

int
MYSQLPool::getClientBand(unsigned int id){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    int result;
	try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"SELECT  availableBand FROM client_info where client_id = '%d'",id);
        res = stmt->executeQuery(sql);
        while(res->next()){
            result = res->getUInt("availableBand");
        }
        delete res;
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        result = -1;
    }
    releaseConnection(con);
	return result;
}
	
int
MYSQLPool::insertP2pClient(unsigned int id,unsigned int maxBand,unsigned int availableBand){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
		cout<<"maxBand = "<<maxBand<<",availableBand = "<<availableBand<<endl;
        snprintf(sql,MAXLINE,"INSERT INTO client_info VALUES('%d',NULL,'%d','%f')",
                id,maxBand,(double)availableBand);
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
	
	return retCode;
}
int
MYSQLPool::updateP2pClient(unsigned int clientId,unsigned int currentBand){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"update client_info set availableBand = '%d' where client_id = '%d'",
                currentBand,clientId);
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
	return retCode;
}
int
MYSQLPool::insertFileSegment(unsigned int fileId,unsigned int segId,unsigned int clientId,unsigned int size){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"INSERT INTO ivod_seg_t VALUES('%d','%d','%d','%d')",
                fileId,segId,clientId,size);
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
	return retCode;
}

int
MYSQLPool::deleteFileSegment(unsigned int fileId,unsigned int segId,unsigned int clientId){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"DELETE FROM ivod_seg_t WHERE client_id = '%d' and segNumber = '%d' and id = '%d'",clientId,segId,fileId);
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
    return retCode;

}

int
MYSQLPool::getFileSegInfo(unsigned int fileId,unsigned int segId,unsigned int &clientId){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    int retCode = 0;
    try{
		con = getConnection();
        stmt = con->createStatement();
    
		snprintf(sql,MAXLINE,"select client_info.client_id from client_info , ivod_seg_t where client_info.client_id = ivod_seg_t.client_id  and id ='%d' and segNumber ='%d'  order by availableBand desc limit 1",fileId,segId);
		res = stmt->executeQuery(sql);
		if(res->rowsCount()==0){//have no result
			delete res;
	        delete stmt;
			releaseConnection(con);
			return -1;
		}
        while(res->next()){
			clientId = res->getUInt("client_id");
	//		cout<<"clientId = "<<clientId<<endl;
        }
        delete res;
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
	return retCode;
}

int
MYSQLPool::resetClient(){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"DELETE FROM client_info");
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
	return retCode;
}

int 
MYSQLPool::resetClientSegment(){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"DELETE FROM ivod_seg_t");
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
	return retCode;

}

int
MYSQLPool::deleteAllResources(){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"DELETE FROM ivod_video_t");
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
	return retCode;

	
}

int
MYSQLPool::deleteClient(unsigned int clientId){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"DELETE FROM client_info  WHERE id = '%d'",clientId);
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
    return retCode;
}
int
MYSQLPool::deleteClientBuf(unsigned int clientId){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    int retCode = 0;
    try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"DELETE FROM ivod_seg_t WHERE id = '%d'",clientId);
        stmt->execute(sql);
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        retCode = -1;
    }
    releaseConnection(con);
    return retCode;
}


int
MYSQLPool::getAvailableBand(){
    char sql[MAXLINE];
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    int result;
	try{
        con = getConnection();
        stmt = con->createStatement();
        snprintf(sql,MAXLINE,"SELECT sum(availBand) as totalLoad FROM client_info ");
        res = stmt->executeQuery(sql);
        while(res->next()){
            result = res->getUInt("totalLoad");
        }
        delete res;
        delete stmt;
    }catch(sql::SQLException &e){
        cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__<<endl
            <<"Description :"<<e.what()<<endl
            <<"SQL :"<<sql<<endl;
        result = -1;
    }
    releaseConnection(con);
	return result;
}




