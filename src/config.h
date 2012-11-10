/*the configure file for server and client
 */
#ifndef CONFIG_H
#define CONFIG_H
#include<map>
#include<string>
#include<iostream>
using namespace std;

enum BufManagerStrategy{
	LRU,
	LFU,
	LRFU,
	PLFU
};


typedef struct server_config{
	unsigned int resourceNumber;
	unsigned int clientNumber;
	unsigned int  minBitRate;
	unsigned int maxBitRate;
	unsigned long minLen;
	unsigned long maxLen;
	unsigned int blockSize;
	unsigned int maxLoad;
	unsigned int bandwidth;
	unsigned int clientBandwidth;
	string  serverPort;
	string managerPort;
	string dbFileName;
	string statisticalResultFile;
	unsigned int statisticalTimeInterval;
	bool isP2pOpen;
	bool needCreateResources;
}ServerConfigType;


typedef struct client_config{
	double poissonLambda;
	unsigned int clientNumber;
	unsigned int resourceNumber;
	unsigned int clientTypes;
	double 	zipfParameter ;
	string serverPort;
	string managerPort;
	string serverIpAddress;
	string bufManagerStrategy;
	string circleConfigFile;
	string requestListFile;
	string bufRecordFile;
	unsigned int clientBandwidth;
	unsigned int totalBufSize;
	unsigned int blockSize;
	unsigned int period;
	unsigned int lrfuLambda;
	unsigned int infMaxLoad;
	unsigned int subMaxLoad;
	unsigned int prefetchNumber;
}ClientConfigType;

int parse_config_line(const string &line,string &key,string &value );
int read_server_config(const string filename , ServerConfigType &serverConfig);
int read_client_config(const string filename, ClientConfigType &);
int read_clientcircle_config(const string filename,double &zeta,double &sigma);
int read_config_file(const string fileName,map<string,string>& conf_map);

#endif
