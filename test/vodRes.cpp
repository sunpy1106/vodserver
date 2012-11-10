#include "vodRes.h"

vodRes::vodRes(int c,int s,int v){
	capacity = c;
	servers = s;
	videos = v;
}

bool comp(const struct vodInfo &vod1,const struct vodInfo &vod2){
	if(vod1.probability > vod2.probability){
		return true;
	}else if( fabs(vod1.probability - vod2.probability ) < infinitesimal ){
		if(vod1.vodId < vod2.vodId)
			return true;
	}
	return false;
}

int 
vodRes::AdamMonotome(){
	struct vodInfo vod;
	for(int i=0;i<videos;i++){
		vod.vodId = i;
		vod.probability = distribution[i]; 
		vod.copys = 1;
		placement.push_back(vod);
	}
	int replications,curVod,maxRep,curIndex;
	replications = videos;
	maxRep = servers * capacity;
	curIndex = curVod = 0;
	while( replications <= maxRep){
		double maxP = placement[curIndex].probability;
		for(int i=0;i<curIndex;i++){						// find the max probability
			if(placement[i].probability > maxP){
				maxP = placement[i].probability;
				curVod=  placement[i].vodId ;
			}
		}
		for(int i=0;i<placement.size();i++){
			if(placement[i].vodId != curVod)
				continue;
			placement[i].copys ++ ;
			placement[i].probability =  (placement[i].copys - 1) * placement[i].probability;
			placement[i].probability = placement[i].probability / placement[i].copys;

		}
		vod.vodId = curVod;
		vod.copys = placement[curVod].copys;
		vod.probability = placement[curVod].probability;
		placement.push_back(vod);
		replications ++;
		if(curVod  == curIndex)
			curIndex ++;
	}
	sort(placement.begin(),placement.end(),comp);
	return 0;
}

int 
vodRes::setDistribution(const vector<double> & arg){
	distribution = arg;
	return 0;
}


int
vodRes::zipfBasedReplication(double skew,vector<int> &replication){
	double max,min;
	max = skew * log2(videos) +  log2(servers);
	min = 0 - ( max /(log2(servers) - log2(servers - 1)));
	int sum;
	sum = getZipfInterval(skew,replication);
	if(sum <= servers * capacity)
		return  0;
	double start,end,cur,minInterval;
	minInterval = pow(videos,-2);
	start = max;
	end  = min;
	while(start - end > minInterval){
		cur = (start + end )/2;
		sum = getZipfInterval(skew,replication);
		if(sum == servers*capacity)
			return 0;
		else if(sum > servers * capacity){
			start = cur;
		}else
			end = cur;
	}
	return 0;
}

int
vodRes::getZipfInterval(double skew,vector<int> &replication){
	vector<double> intervals;
	double denominator = 0.0;
	for(int i=0;i<servers;i++){				// get the sum of pow(i+1,-skew);
		denominator += pow(i+1,0-skew);
	}
	double u,pm1;
	pm1 = distribution[0] + distribution[videos-1];
	for(int i=0;i<servers;i++){				// get the u1,u2,u3,...
		u = pm1 * pow(i+1,0-skew);
		u = u / denominator;
		intervals.push_back(u);
	}
	intervals.push_back(0.0);
	if(replication.size()!=0)				//get the r1,r2,....
		replication.clear();
	int j,size,sum;
	sum = j = 0;
	size = intervals.size();
	for(int i=0;i<servers;){
		if(distribution[i] < intervals[j]){
			if(distribution[i] > intervals[j+1]){
				sum += size -j;
				replication.push_back(size-j);
				i++;
			}else{
				j++;
			}
		}
	}
	return sum;
}

int 
vodRes::MMPacking(vector<double> spread){
	double max = 1.0/servers;
	vector<double> smp;
	for(int i=0;i<servers;i++){
		smp.push_back(0.0);	
	}
	int curVod,curMachine=0;
	struct vodInfo curVodInfo;
	for( curVod = 0;curVod < spread.size();){
		if( (smp[curMachine] + spread[curVod] -  max )  < infinitesimal   ){
			smp[curMachine] += spread[curVod];
			curVodInfo.vodId = curVod;
			curVodInfo.machineId = curMachine;
			curVodInfo.probability = spread[curVod];
			placement.push_back(curVodInfo);
			curVod ++;
		}else {
			while(1){
				curVodInfo.vodId = curVod;
				curVodInfo.machineId = curMachine;
				curVodInfo.probability = max - smp[curMachine];
				placement.push_back(curVodInfo);
				spread[curVod] = smp[curMachine] +  spread[curVod] - max;
				smp[curMachine ] = max;
				curMachine = (curMachine + 1)% servers;
				if( (smp[curMachine] + spread[curVod] -  max )  < infinitesimal   )
					break;
			}
		}
	}
	return 0;
}


