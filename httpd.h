 #ifndef __HTTPD__H__
#define __HTTPD__H__
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#define SUCCESS 0
#define NOTICE 1
#define WARNING 2
#define ERROR 3
#define FATAL 4
#define SIZE 1024




int startup(const char* ip,int port);
void print_log( char *msg,int level);
void *headler_request(void *arg);

#endif
         
