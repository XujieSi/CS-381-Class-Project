
#include <cstdio>
#include <iostream>
using namespace std;

int packet[100];

char buf[1024];

int main()
{
	int numOfNodes;
	int appDR,csmaDR;
	int totalPackets=0;
	double max_time;
	int line_num=0;

	printf("NumOfNodes;Application DataRate(bps);CsmaChannel DataRate(bps);Packets;FinishedTime(s)\n");

	while(scanf("%s%d%s%d%s%d",buf,&numOfNodes,buf,&appDR,buf,&csmaDR)!=EOF){
		line_num++;
		int temp;
		scanf("%d%s",&temp,buf);
		if(temp!=numOfNodes){
			printf("error(line %d): temp=%d numOfNodes=%d\n",line_num,temp,numOfNodes);
			return 0;
		}

		totalPackets=0;
		for(int i=0;i<numOfNodes;++i){
			int id,en,send,recv,drop;
			scanf("%s%d%s%d%s%d%s%d%s%d",buf,&id,buf,&en,buf,&send,buf,&recv,buf,&drop);
			totalPackets += send;
		}
		scanf("%s%lf",buf,&max_time);
		printf("%d;%d;%d;%d;%lf\n",numOfNodes,appDR,csmaDR,totalPackets,max_time);
	}
	

	return 0;
}
