/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  server_epoll2.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/31/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/31/23 11:02:21"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <libgen.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sqlite3.h>
#include <signal.h>
#include "zlog.h"

#define maxn 100
#define BACKLOG 13
#define MAX_EVENTS      512
#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))

int g_stop = 0;
static sqlite3 *db;

void print_usage(char *progname);
int server_start(char *host,int port);
void set_socket_rlimit(void);
int callback(void *arg,int column,char **value,char **name);
sqlite3 *sql_conn(char *db_name);
void sql_create(void);
void sql_insert(void);
void sql_get_table(void);
void sig_handle(int signum);

int main(int argc, char **argv)
{
	int ch;
	int rv;
	int i,j;
	int events;
	int epollfd;
	int conn_fd;
	int listen_fd;
	int port = 0;
	int found = 0;
	int maxfd = 0;
	char buf[1024];
	char *db_name ="test.db";
	zlog_category_t *zc;

	struct epoll_event  event;
	struct epoll_event  event_array[MAX_EVENTS];

	struct option opts[] = {
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while((ch = getopt_long(argc, argv, "p:h", opts, NULL)) != -1 )
	{
		switch(ch)
		{
			case 'p':
				port = atoi(optarg);
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if(!port)
	{
		printf("please input the port!\n");
		print_usage(argv[0]);
		return -1;
	}

	set_socket_rlimit();//rlimit the number of socket

	/* create zlog for server */
	if((zlog_init("zlog.conf")))
	{
		printf("zlog init failure\n");
		return -1;
	}

	zc = zlog_get_category("my_cat");
	if(!zc)
	{
		printf("zlog get category failure\n");
		zlog_fini();
		return -1;
	}
	zlog_info(zc,"zlog get category successfully");

	if((listen_fd = server_start(NULL,port)) < 0 )
	{
		zlog_error(zc,"create socket failure: %s",strerror(errno));
		return -1;
	}
	zlog_info(zc,"server start to listen on the port[%d]",port);

	if((epollfd = epoll_create(MAX_EVENTS)) < 0)
	{
		zlog_error(zc,"epoll_create() failure:%s",strerror(errno));
		return -1;
	}

	event.events = EPOLLIN;
	event.data.fd = listen_fd;

	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,listen_fd,&event) < 0)
	{
		zlog_error(zc,"epoll add listen socket failure:%s",strerror(errno));
		return -1;
	}

	while(!g_stop)
	{
		signal(SIGINT,sig_handle);
		events = epoll_wait(epollfd,event_array,MAX_EVENTS,-1);
		if(events < 0 )
		{
			zlog_error(zc,"epoll failure:%s",strerror(errno));
			break;
		}
		else if (events == 0)
		{
			zlog_error(zc,"epoll get timeout");
			continue;
		}

		for(i=0; i<events; i++)
		{
			if((event_array[i].events&EPOLLERR) || (event_array[i].events&EPOLLHUP))
			{
				zlog_error(zc,"epoll_wait get error on fd[%d]:%s",event_array[i].data.fd,strerror(errno));
				epoll_ctl(epollfd,EPOLL_CTL_DEL,event_array[i].data.fd,NULL);
				close(event_array[i].data.fd);
			}

			if(event_array[i].data.fd == listen_fd)
			{
				if((conn_fd = accept(listen_fd,(struct sockaddr *)NULL,NULL)) < 0)
				{
					zlog_error(zc,"accept new client failure:%s",strerror(errno));
					continue;
				}

				event.data.fd = conn_fd;
				event.events = EPOLLIN;
			
				if( epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_fd,&event) < 0 )
				{
					zlog_error(zc,"epoll add client socket failure:%s",strerror(errno));
					close(event_array[i].data.fd);
					continue;
				}
				zlog_info(zc,"epoll add client socket[%d] successfully",conn_fd);
			}
			else
			{
				if(( rv = read(event_array[i].data.fd,buf,sizeof(buf))) <= 0)
				{
					zlog_error(zc,"socket[%d] read failure or get disconnect and will be removed",event_array[i].data.fd);
					epoll_ctl(epollfd,EPOLL_CTL_DEL,event_array[i].data.fd,NULL);
					close(event_array[i].data.fd);
					continue;
				}
				else
				{
					db=sql_conn(db_name);
					sql_create();
					zlog_info(zc,"socket[%d] read get %d bytes data",event_array[i].data.fd,strlen(buf));
					sql_insert();
					sql_get_table();

					for(j=0; j<rv; j++)
					{
						buf[j] = toupper(buf[j]);
					}

					if(write(event_array[i].data.fd,buf,rv) < 0)
					{
						zlog_error(zc,"socket[%d] write failure:%s",strerror(errno));
						epoll_ctl(epollfd,EPOLL_CTL_DEL,event_array[i].data.fd,NULL);
						close(event_array[i].data.fd);
					}
				}
			}
		}
	}

	printf("\n");
	zlog_fini();
	close(listen_fd);
	return 0;
}

void print_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("-p(--port):sepcify server port.\n");
	printf("-h(--Help):print this help information.\n");
	return ;
}

int server_start(char *host,int port)
{
	int  on = 1;
	int  listen_fd = -1;
	socklen_t  len;
	struct sockaddr_in servaddr;

	if((listen_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
	{
		printf("create socket failure:%s.\n",strerror(errno));
		return -1;
	}

	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	if(bind(listen_fd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
	{
		printf("bind socket failure:%s\n",strerror(errno));
		return -1;
	}

	if(listen(listen_fd,BACKLOG) < 0 )
	{
		printf("listen failure:%s\n",strerror(errno));
		return -1;
	}

	sqlite3_close(db);
	return listen_fd;
}

void set_socket_rlimit(void)
{
	struct rlimit limit = {0};
	getrlimit(RLIMIT_NOFILE,&limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE,&limit);
	printf("set socket open fd max count to %d\n",limit.rlim_max);
}

sqlite3 *sql_conn(char *db_name)
{
	int ret;
	ret = sqlite3_open(db_name,&db);
	if(ret != SQLITE_OK)
	{
		printf("sql open failure:%s\n",sqlite3_errmsg(db));
	}
	printf("open sql successfully\n");
	return db;
}

void sql_create(void)
{
	int ret;
	char *error;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	sprintf(sql,"create table if not exists serv_data(""time char(50),"
													"temp char(50),"
													"chip char(20));");
	
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		printf("create table failure:%s\n",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);
	}
	printf("create table successfully\n");
}

void sql_insert(void)
{
	int ret;
	char *error;
	char sql[maxn] = {0};
	memset(sql,0,sizeof(sql));
	sprintf(sql,"insert into serv_data select * from data;");
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		printf("insert failure:%s\n",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);
	}
}

void sql_get_table(void)
{
	int i;
	int j;
	int ret;
	int row;
	int value;
	int column;
	char *error;
	char **dbResult;
	char sql[maxn] = {0};
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from data");
	ret = sqlite3_get_table(db,sql,&dbResult,&row,&column,&error);
	if(ret != SQLITE_OK)
	{
		printf("get table failure:%s\n",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);
	}
	
	value = column;
	for(i=0; i<row; i++)
	{
		for(j=0; j<column; j++)
		{
			printf("%s",dbResult[value]);
			value++;
		}
		printf("\n--------------------\n");
	}

	sqlite3_free_table(dbResult);
	sqlite3_close(db);
}

void sig_handle(int signum)
{
	if (SIGINT == signum)
	{
		g_stop = 1;
	}
}
