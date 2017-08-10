#include "httpd.h"


static void Usage(const char* proc)
{
	printf("Usage:\n\t%s [local_ip][local_port]\n\n",proc); 
}

int main(int argc,char* argv[])
{
    if(argc!=3)
	{
		Usage(argv[0]);
		return 1;
	}

	int listen_sock=startup(argv[1],atoi(argv[2]));
	while(1)
	{
		struct sockaddr_in client;
		socklen_t len=sizeof(client);
		int rw_sock=accept(listen_sock,(struct sockaddr*)&client,&len);
		if(rw_sock<0)
		{
		    print_log(strerror(errno),NOTICE);
			continue;
		}
		printf("get a client : %s:%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		pthread_t id;
		int ret=pthread_create(&id,NULL,headler_request,(void *)rw_sock);
		if(ret!=0)
		{
			print_log(strerror(errno),NOTICE);
			close(rw_sock);
		}
		else
		{
			pthread_detach(id);
		}
	}
	close(listen_sock);
	return 0;
}
