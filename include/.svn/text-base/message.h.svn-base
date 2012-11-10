#ifndef MESSAGE_H
#define MESSAGE_H

#define SERVERID 1111111111

enum MessageType{
	REQUESTRESOURCE = 0,
	REQUESTSEGMENT,
	REQUESTSEGMENTAGAIN,
	HEARTBEAT,
	BLOCKADD,
	BLOCKELIMINATE,
	CLIENTQUIT,
	ACK,
	RETRANS
};	

/*
 *the packet send to the server when the client require the information
 *of a video ,the message type must be REQUSTRESOOURCE
 */
typedef struct videoInfoReq{
	unsigned int messageType;
	unsigned int clientId;
	unsigned int resourceId;
	unsigned int bandwidth;
}VideoInfoReq;

typedef struct AckMessage{
	unsigned int messageType;
	unsigned int ackMessageType;
}MessageAck;
/*
 *the information of a video
 */

typedef struct videoInfo{
	unsigned int resourceId;
	unsigned long fileLength;
	float bitRate;
}VideoInfo;

/*
 *the packet that send to the server when a client want to get the best service 
 *clients of some segment
 */
typedef struct vodSegReq{
	unsigned int messageType;
	unsigned int clientId;
	unsigned int resourceId;
	unsigned int sidStart;
	unsigned int sidNum;
}VodSegReq;


typedef struct vodSeg{
	unsigned int resourceId;
	unsigned int sid;
	unsigned int clientId;
}VodSeg;	

/*
 *send this packet when the client send heartbeat(HEARTBEAT) or exit(CLIENTQUIT)
 */
typedef struct clientState{
	unsigned int messageType;
	unsigned int clientId;
	unsigned int availableBand;
}ClientState;

typedef struct BufUpdatePack{
	unsigned int messageType;
	unsigned int clientId;
	unsigned int fileId;
	unsigned int  segId;
	unsigned int blockSize;
}BufUpdatePack;


/********************the packet that unused*****************************/

/*
 *the reply packet of vodSegReq,the message type
 *must be REQUESTSEGMENTACK
 */
typedef struct vodSegAck{
	unsigned int messageType;
	unsigned int sidNum;
	VodSeg *vodSegInfo;
}VodSegAck;


/*
 *the packet that server used to reply the client's REQUESTSOURCE packet,
 *the message type must be REQUESTRESOURCEACK
 */
typedef struct videoInfoAck{
	unsigned int messageType;
	unsigned int clientId;
	VideoInfo  videoInfo;
}VideoInfoAck;

#endif

