
#include <cstdio>
#include <iostream>
using namespace std;

int en[1000];
int send[1000];
int recv[1000];
int drop[1000];
char buf[2048];


int main()
{
	int id;
	double tm;
	double max_tm=0.0;
	char c;
	int max_id=0;
	int line_num=0;
	while( scanf("%c %lf /NodeList/%d",&c,&tm,&id) ==3){
		gets(buf);
		line_num++;
		switch(c){
			case '+': en[id]++;break;
			case '-': send[id]++;break;
			case 'r': recv[id]++;break;
			case 'd': drop[id]++;break;
			default : printf("line %d:packet format is wrong!\n",line_num);
		}
		if(max_id<id) max_id=id;
		if(max_tm<tm) max_tm=tm;
	}

	printf("%d nodes\n",max_id+1);
	for(int i=0;i<=max_id;++i){
		printf("Node %d en %d send %d recv %d drop %d\n",i,en[i],send[i],recv[i],drop[i]);
	}
	printf("max_time: %lf\n",max_tm);

	return 0;
}


