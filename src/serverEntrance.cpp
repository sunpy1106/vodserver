#include "server.h"
#include "config.h"

int main(int argc,char *argv[]){
	ServerConfigType serverConfig;
	string fileName("config/server.ini");
	read_server_config(fileName,serverConfig);
	Server *ser  = new Server(&serverConfig);
//	ser->printDataMember();
	ser->initServer();
	return 0;
}
