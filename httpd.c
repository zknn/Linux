#include "httpd.h"
#define SERVER_STRING "Server:jdbhttpd/1.0\r\n"

void print_log( char* msg,int level)
{
#ifdef _STDOUT_
	const  char * const  level_msg[]=
	{
		"SUCCESS",
		"NOTICE",
		"WARNING",
		"ERROR",
		"FATAL",
	};
	printf("[%s] [%s] \n",msg,level_msg[level%5]);
#endif
}
static  int get_line(int sock,char line[],int size)
{
	char c='\0';
	int len=0;
	while(c!='\n'&&len<size-1)
	{
		int r=recv(sock,&c,1,0);
		if(r>0)
		{
			if(c=='\r')
			{
				//窥探
				int ret=recv(sock,&c,1,MSG_PEEK);
				if(ret>0)
				{
					if(c=='\n')
					{
						recv(sock,&c,1,0);
					}
					else
					{
						c='\n';
					}
		    	}
            }
			line[len++]=c;
		}
		else
		{
			c='\n';
		}
	}
	line[len]='\0';
	return len;
}
//
////bad_request
//static void bad_request(int client)
//{
//		char buf[1024];		
//		sprintf(buf,"HTTP/1.0 400 BAD REQUEST\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"Countent-type: text/html\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"\r\n");
//        send(client,buf,strlen(buf),0);
//		sprintf(buf,"<P>your enter message is bad request</p>\r\n");
//    	send(client,buf,strlen(buf),0);
//	    sprintf(buf,"such as a POST without a Content-Length.\r\n");
//		send(client,buf,strlen(buf),0);							
//}
//static void cannot_execute(int client)
//{
//		char buf[1024];
//	    sprintf(buf,"HTTP/1.0 401 CANNOT EXECUTE\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"Countent-type: text/html\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"<P>Cgi message is bad  </p>\r\n");
//		send(client,buf,strlen(buf),0);							
//}
////404
//static void not_found(int client)
//{	
//		char buf[1024];
//	   sprintf(buf,"HTTP/1.0 404 NOT FOUND\r\n");
//	   send(client,buf,strlen(buf),0);
//    	sprintf(buf,SERVER_STRING);
//		send(client,buf,sizeof(buf),0);
//		sprintf(buf,"Countent-type: text/html\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"<HTML><TITLE>NOT FOUND</TITLE>\r\n");
//	    send(client,buf,strlen(buf),0);
//		sprintf(buf,"<BODY><P>The server could not fulfill\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"your request because the resource \r\n\
//		specifild isunavailable or nonexistent.\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"</BODY></HTML>\r\n");
//		send(client,buf,strlen(buf),0);
//}
//static void server_unavibale(int client)
//{
//		char buf[1024];
//		sprintf(buf,"HTTP/1.0 501  METHOND NOT Implemented\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,SERVER_STRING);
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"Countent-Type: text/html\r\n");
//	    send(client,buf,strlen(buf),0);
//		sprintf(buf,"\r\n");
//		send(client,buf,strlen(buf),0);
//	    sprintf(buf,"<HTML><HEAD><TITLE>Method Not Implemented</TITLE></HEAD>\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"<BODY><P>HTTP request method not supported.\r\n");
//		send(client,buf,strlen(buf),0);
//		sprintf(buf,"</BODY></HTML>\r\n");
//	    send(client,buf,strlen(buf),0);
//}
//void echo_string(int client, int error_code)
//{
//		switch(error_code){
//		case 400:
//		bad_request(client);
//		break;
//		case 401:
//	   cannot_execute(client);
//		break;
//		case 404:
//		not_found(client);
//	    break;
//		case 501:
//		server_unavibale(client);
//		break;
//		default:									
//		break;
//		}
//}
int startup(const char* ip,int port)
{
  int sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock<0)
  {
  	print_log(strerror(errno),FATAL);
  	exit(2);
  }
  int opt=1;
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
  struct sockaddr_in local;
  local.sin_family=AF_INET;
  local.sin_port=htons(port);
  local.sin_addr.s_addr=inet_addr(ip);
  if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
  {
  	print_log(strerror(errno),FATAL);
  	exit(3);
  }
  if(listen(sock,10)<0)
  {
  	print_log(strerror(errno),FATAL);
  exit(4);
  }
  return sock;
}
static void echo_string(int sock,int err)
{}
//
 static int echo_www(int sock,char* path,int size)
{
	int fd=open(path,O_RDONLY);
	if(fd<0)
	{
		echo_string(sock,404);
		print_log(strerror(errno),FATAL);
		return 8;
	}
	const char *echo_line="HTTP/1.0 200 OK\r\n";
	send(sock,echo_line,strlen(echo_line),0);
	const char *null_line="\r\n";
	send(sock,null_line,strlen(null_line),0);
	if(sendfile(sock,fd,NULL,size)<0)
	{
		echo_string(sock,404);
		print_log(strerror(errno),FATAL);
		return 9;
	}
	close(fd);
	return 0;
}

static void drop_header(int sock)
{
	char line[1024];
	int ret=-1;
	do
	{
		ret=get_line(sock,line,sizeof(line));
	}while(ret>0&&strcmp(line,"\n"));
}
static int exe_cgi(int sock,char *method,  char *path,char *query_string)
{
	int content_len=-1;
	char method_env[SIZE/10];
    char query_string_env[SIZE];
	char content_len_env[SIZE/10];
	if(strcasecmp(method,"GET")==0)
	{
		drop_header(sock);
	}
	else
	{
	    char line[1024];
		int ret=-1;
		do
		{
			ret=get_line(sock,line,sizeof(line));
			if(ret>0&&strncasecmp(line,"Content-Lenght: ",16)==0)
			{
				content_len=atoi(&line[16]);
			}
		}while(ret>0&&strcmp(line,"\n"));
		if(content_len==-1)
		{
			echo_string(sock,400);
			return 10;
		}
	}

	const char *echo_line="HTTP/1.0 200 OK\r\n";
	send(sock,echo_line,strlen(echo_line),0);
	const char*type="Content-Type:text/html;charset=IS0-8859-1\r\n";
	send(sock,type,strlen(type),0);
	const char *null_line="\r\n";
	send(sock,null_line,strlen(null_line),0);
	printf("query_string:%s\n",query_string);
	int input[2];
	int output[2];
	if(pipe(input)<0||pipe(output)<0)
	{
		echo_string(sock,401);
		return 11;
	}

	pid_t id=fork();
	if(id<0)
	{
		echo_string(sock,401);
		return 12;
	}
	else if(id==0)
	{
		close(input[1]);
		close(output[0]);
		sprintf(method_env,"METHOD=%s",method);
		putenv(method_env);
		if(strcasecmp(method,"GET")==0)
		{
			sprintf(query_string_env,"QUERY_STRING=%s",query_string);
			putenv(query_string_env);
		}
		else
		{
			sprintf(content_len_env,"CONTENT_LENGTH=%d",content_len);
			putenv(content_len_env);
		}
    	dup2(input[0],0);
		dup2(output[1],1);
		execl(path,path,NULL);
		printf("execl error\n");
		exit(1);
	}
	else
	{
		close(input[0]);
		close(output[1]);

		int i=0;
		char c='\0';
		if(strcasecmp(method,"POST")==0)
    	{ 
		for(;i<content_len;i++)
		{
			recv(sock,&c,1,0);
			write(input[1],&c,1);
		}
    	}
	c='\0';
	while(read(output[0],&c,1)>0)
	{
	  send(sock,&c,1,0);
	}
       waitpid(id,NULL,0);
		close(input[1]);
		close(output[0]);
	}
}
//

void* headler_request(void* arg)
{
	int sock=(int)arg;
#ifdef _DEBUG_
	char line[1024];
	do
	{
		int ret=get_line(sock,line,sizeof(line));
		if(ret>0)
		{
			printf("%s",line);
		}
		else
		{
			printf("request...done!\n");
			break;
		}
	}while(1);
#else
	int ret=0;
	char buf[SIZE];
	char method[SIZE/10];
	char url[SIZE];
	int i,j;
	int cgi=0;
	char* query_string=NULL;
	char path[SIZE];
	if(get_line(sock,buf,sizeof(buf))<=0)
	{
		echo_string(sock,501);
		ret=5;
		goto end;
	}
	i=0;  //method ->index
	j=0;  //buf ->index
	while(!isspace(buf[j])&&i<sizeof(method)-1&&j<sizeof(buf))
	{
		method[i]=buf[j];
		i++;
		j++;
	}
	method[i]=0;
	if(strcasecmp(method,"GET")&&strcasecmp(method,"POST"))
	{
	    echo_string(sock,501);
		ret=6;
		goto end;
	}
	//buf ->"GET"   /http/1.0
	while(isspace(buf[j])&&j<sizeof(buf))
	{
		j++;
	}
	i=0;
	while(!isspace(buf[j])&&j<sizeof(buf)&&i<sizeof(url)-1)
	{
		url[i]=buf[j];
		i++;
		j++;
	}
	url[i]=0;
	printf("method:%s,url:%s\n",method,url);
    query_string=url;
	while(*query_string !='\0')
	{
		if(*query_string=='?')
		{
			*query_string='\0';
			query_string++;
			cgi=1;
			break;
		}
		query_string++;
	}
	sprintf(path,"wwwroot%s",url);
	if(path[strlen(path)-1]=='/')
	{
	//	sprintf(path,"%sindex.html",url);
	strcat(path,"index.html");
	}
	struct stat st;
	if(stat(path,&st)!=0)
	{
		echo_string(sock,501);
		ret=7;
		goto end;
	}
	else
	{
		if(S_ISDIR(st.st_mode))
		{
			strcat(path,"/index.html");
		}
		else if((st.st_mode&S_IXUSR) || \
			   (st.st_mode& S_IXGRP) || \
			   (st.st_mode& S_IXOTH))
			   {
				cgi=1;
			   }
			   else
			   {}
			   if(cgi)
			   {
				   printf("enter CGI\n");
				   exe_cgi(sock,method,path,query_string);
			   }
			   else
			   {
				   printf("method:%s,url:%s,path:%s,cgi:%d,query_string:%s \n",method,url,path,cgi,query_string);		   
				   drop_header(sock);
				   echo_www(sock,path,st.st_size);  
			   }
	}
	end:
	printf("quit client...");
	close(sock);
   return (void*)ret;   
#endif
}
