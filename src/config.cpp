#include "config.h"

#include "../include/global.h"

int parse_config_line(const string &line,string &key,string& value){
	size_t pos = 0;
	string lineBuf = line;
	while((pos = lineBuf.find(' '))!= string::npos){
		lineBuf.erase(pos,1);
	}
	while((pos = lineBuf.find('\t'))!= string::npos){
		lineBuf.erase(pos,1);
	}

	if(lineBuf[0] == '#')
		return -1;
	pos = lineBuf.find('#');
	if(pos != string::npos){
		lineBuf = lineBuf.substr(0,pos);
	}
	pos = lineBuf.find('=');
	if(pos != string::npos){
		key = lineBuf.substr(0,pos);
		value = lineBuf.substr(pos +1);
	}
	return 0;

}
int read_server_config(const string fileName,ServerConfigType &serverConfig){
    ifstream confFile( fileName.c_str() );

    if ( !confFile.is_open() ) {
        cout << "\tFatal Error : Cannot open configure file "
             << fileName << endl;
        exit(-1);
    }

    string line;
	cout<<"read config file "<<fileName<<endl;
    while( confFile.good() ) {
        getline( confFile, line );
		string key,value;
        parse_config_line( line ,key,value);
		if(key.size() ==0 || value.size()==0)
			continue;
		cout<<"set "<<key<<" = "<<value<<endl;
		if(key.compare("resourceNumber") == 0){
			serverConfig.resourceNumber = atoi(value.c_str());
		}else if(key.compare("clientNumber") == 0){
			serverConfig.clientNumber = atoi(value.c_str());
		}else if(key.compare("maxBitRate") == 0){
			serverConfig.maxBitRate = atoi(value.c_str());
		}else if(key.compare("minBitRate") == 0){
			serverConfig.minBitRate = atoi(value.c_str());
		}else if(key.compare("minFileLen") == 0){
			serverConfig.minLen = atoi(value.c_str());
		}else if(key.compare("maxFileLen") == 0){
			serverConfig.maxLen = atoi(value.c_str());
		}else if(key.compare("isP2pOpen") == 0){
			serverConfig.isP2pOpen = (value == "true"?true:false);
		}else if(key.compare("needCreateResources") == 0){
			serverConfig.needCreateResources = (value == "true"?true:false);
		}else if(key.compare("blockSize") == 0){
			serverConfig.blockSize = atoi(value.c_str());
		}else if(key.compare("bandwidth") == 0){
			serverConfig.bandwidth = atoi(value.c_str());
		}else if(key.compare("clientBandwidth") == 0){
			serverConfig.clientBandwidth = atoi(value.c_str());
		}else if(key.compare("serverPort") == 0){
			serverConfig.serverPort = value;
		}else if(key.compare("managerPort") == 0){
			serverConfig.managerPort = value;
		}else if(key.compare("dbFileName") == 0){
			serverConfig.dbFileName = value;
		}else if(key.compare("statisticalTimeInterval") == 0){
			serverConfig.statisticalTimeInterval = atoi(value.c_str());
		}else if(key.compare("statisticalResultFile") == 0){
			serverConfig.statisticalResultFile = value;
		}else{
			cout<<"can't find  arg :"<<key<<endl;
		}

    }
    confFile.close();
	return 0;
}

int read_client_config(const string fileName,ClientConfigType &clientConfig){
    ifstream confFile( fileName.c_str() );

    if ( !confFile.is_open() ) {
        cout << "\tFatal Error : Cannot open configure file "
             << fileName << endl;
        exit(-1);
    }

    string line;
	cout<<"read config file "<<fileName<<endl;
    while( confFile.good() ) {
        getline( confFile, line );
		string key,value;
        parse_config_line( line ,key,value);
		if(key.size() ==0 || value.size()==0)
			continue;
		cout<<"set "<<key<<" = "<<value<<endl;
		if(key.compare("poissonLambda") == 0){
			clientConfig.poissonLambda = atoi(value.c_str())/1000.0;
		}else if(key.compare("zipfParameter") == 0){
			clientConfig.zipfParameter = atoi(value.c_str())/1000.0;
		}else if(key.compare("lrfuLambda") == 0){
			clientConfig.lrfuLambda = atoi(value.c_str())/1000.0;
        }else if(key.compare("resourceNumber") == 0){
			clientConfig.resourceNumber = atoi(value.c_str());
		}else if(key.compare("clientNumber") == 0){
			clientConfig.clientNumber  = atoi(value.c_str());
		}else if(key.compare("clientTypes") == 0){
			clientConfig.clientTypes = atoi(value.c_str());
		}else if(key.compare("clientBandwidth") == 0){
			clientConfig.clientBandwidth = atoi(value.c_str());
		}else if(key.compare("serverIpAddress") == 0){
			clientConfig.serverIpAddress = value;
		}else if(key.compare("serverPort") == 0){
			clientConfig.serverPort = value;
		}else if(key.compare("managerPort") == 0){
			clientConfig.managerPort = value;
		}else if(key.compare("totalBufSize") == 0){
			clientConfig.totalBufSize = atoi(value.c_str());
		}else if(key.compare("blockSize") == 0){
			clientConfig.blockSize = atoi(value.c_str());
		}else if(key.compare("period") == 0){
			clientConfig.period = atoi(value.c_str());
		}else if(key.compare("bufManagementStrategy") == 0){
			clientConfig.bufManagerStrategy = value;
		}else if(key.compare("circleConfigFile") == 0){
			clientConfig.circleConfigFile = value;
		}else if(key.compare("bufRecordFile") == 0){
			clientConfig.bufRecordFile = value;
		}else if(key.compare("requestListFile") == 0){
			clientConfig.requestListFile = value;

		}else if(key.compare("infMaxLoad") == 0){
			clientConfig.infMaxLoad = atoi(value.c_str());
		}else if(key.compare("subMaxLoad") == 0){
			clientConfig.subMaxLoad = atoi(value.c_str());
		}else if(key.compare("prefetchNumber") == 0){
			clientConfig.prefetchNumber = atoi(value.c_str());
		}else{
			cout<<"can't find this arg :"<<key<<endl;
		}

    }
	confFile.close();
	return 0;
}


int read_clientcircle_config(const string filename,double &zeta,double &sigma){
	return 0;
}



static string& trim(string& s){
    s.erase(0,s.find_first_not_of(" ")); 
    s.erase(s.find_last_not_of(" ") + 1); 
    return s;
}

//read a  file
int read_config_file(const string filename, map<string,string>& conf_map){
    ifstream infile;
    infile.open(filename.c_str());
    if(!infile){
        err_msg("Can not open file %s",filename.c_str());
        return -1;
    }
   
    conf_map.clear();
    
    for(string line,key,value; getline(infile,line);){
        int pos; //position of "="
        trim(line);
        if(line.empty()) continue;
        if(line[0] == '#') continue;
        pos = line.find_first_of('=');
        key = line.substr(0,pos);
        value = line.substr(pos+1,line.length() - pos - 1);
        trim(key);
        trim(value);
        conf_map[key] = value;
    }
    return 0;
}

