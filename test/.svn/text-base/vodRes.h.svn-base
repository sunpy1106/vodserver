#ifndef _VODRES_H
#define _VODRES_H
#include "../include/global.h"

/* this class presents a solid theoretical framewook for video replication and placement  problem;specifically,
 * 1.we f
*/

const double infinitesimal = 0.0000001;

class vodRes{
	public:
		vodRes(int c,int s,int v);
	public:	//replication
		int AdamMonotome();
		int zipfBasedReplication(double skew,vector<int> &replication);
	public:	//placement
		int MMPacking(vector<double> spread);
	public: //schedule

	public: //set value
		int setDistribution(const vector<double> &arg);
	public: //get the result

	private:
		//return the sum of replications and the 
		int getZipfInterval(double skew ,vector<int> &replication);
	private: 
		int capacity;
		int servers;
		int videos;
		vector<double> distribution;  //in desc order
		vector<vodInfo> placement;
};

#endif
