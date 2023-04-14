/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  client_test2.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/29/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/29/23 21:51:42"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include <signal.h>
#include "zlog.h"

#define maxn 100

int g_stop = 0;
static sqlite3 *db;

void printf_usage(char *progname);
int get_temp(float *temp,char *chip);
int get_first_time(long int *t_begin);
int get_second_time(long int *t_end);
int callback(void *arg,int column,char **value,char **name);
sqlite3 *sql_conn(char *db_name);
void sql_create(void);
void sql_insert(char *begin_time,char s_temp[50],char *chip);
void sql_select(void);
void sql_delect(void);
void sig_handle(int signum);
void get_zlog(zlog_category_t *zc);

int main(int argc,char **argv)
{
	int     rv = -1;
	int     ch = 0 ;
	int     port = 0;
	int     tim = 0;
	int 	diff_time = 0;
	int     conn_fd =-1;
	float   temp = 0;
	char	s_temp[50];
	char    str[1024] ;
	char    buf[1024];
	char	chip[50];
	char    ipstr[1024];
	char    **hostip = 0;
	char    *servip = 0;
	char    *hostname = 0;
	char	*db_name = "test.db";
	long int t_begin = 0;
	long int t_end = 0;
	zlog_category_t *zc;

	struct hostent  *servhost;
	struct sockaddr_in servaddr;
	struct tcp_info info;
	int len = sizeof(info);

	struct option  opts[] = {
		{"hostname",required_argument,NULL,'h'},
		{"port",required_argument,NULL,'p'},
		{"time",required_argument,NULL,'t'},
		{"Help",no_argument,NULL,'H'},
		{NULL,0,NULL,0}
	};

	while((ch = getopt_long(argc,argv,"h:p:t:H",opts,NULL)) != -1)
	{
		switch(ch)
		{
			case'h':
				hostname = optarg;
				break;
			case'p':
				port=atoi(optarg);
				break;
			case 't':
				tim = atoi(optarg);
				break;
			case'H':
				printf_usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if((inet_aton(hostname,&servaddr.sin_addr)) == 0)
	{
		if((servhost = gethostbyname(hostname)) == NULL)
		{
			printf("get hostname error: %s\n", strerror(errno));
			return -1;
		}

		switch(servhost->h_addrtype)
		{
			case AF_INET6:
			case AF_INET:
				hostip = servhost->h_addr_list;
				for (; *hostip != NULL; hostip++)
					printf("IP: %s\n",inet_ntop(servhost->h_addrtype,servhost->h_addr,ipstr,sizeof(ipstr)));
				servip = ipstr;
				break;
			default:
				printf("error address!\n");
				break;
		}
	}
	else
	{
		servip = hostname;
	}

	signal(SIGINT,sig_handle);
	while(!g_stop)
	{
	//	signal(SIGINT,sig_handle);

		if((zlog_init("cli_zlog.ini")))
		{
			printf("zlog init failure\n");
			return -1;
		}

		zc = zlog_get_category("m_cli");
		if(!zc)
		{
			printf("zlog get category failure\n");
			zlog_fini();
			return -1;
		}
		zlog_info(zc,"zlog get category successfully");
		
		conn_fd = socket(AF_INET,SOCK_STREAM,0);
		if(conn_fd < 0)
		{ 
			zlog_error(zc,"create socket failure:%s",strerror(errno));
			return -1; 
		} 
		zlog_info(zc,"create socket[%d] successfully",conn_fd);

		memset(&servaddr,0,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		inet_aton(servip,&servaddr.sin_addr);

		db = sql_conn(db_name);
		sql_create();
		sql_delect();

		get_first_time(&t_begin);	//get the time
		char *begin_time = ctime(&(t_begin));

		get_temp(&temp,chip);	//get the temp
		sprintf(s_temp,"%f\n",temp);
		snprintf(buf,sizeof(buf),"time: %stemp: %schip: %s",begin_time,s_temp,chip);

		sql_insert(begin_time,s_temp,chip);
		/* connect server */
		if((connect(conn_fd,(struct sockaddr *)&servaddr,sizeof(servaddr))) >= 0)
		{
			zlog_info(zc,"connect to server[%s : %d] successfully",servip,port);
		}
		/* while disconnect,wait for connecting */
		getsockopt(conn_fd,IPPROTO_TCP,TCP_INFO,&info,(socklen_t *)&len);
		if((info.tcpi_state != TCP_ESTABLISHED))
		{
			zlog_info(zc,"client wait for connecting......");
			while(1)
			{
				if((connect(conn_fd,(struct sockaddr *)&servaddr,sizeof(servaddr))) < 0)
				{
					continue;
				}
				else
				{
					zlog_info(zc,"connect to server successfully again");
					break;
				}
			}
		}

		rv = write(conn_fd,buf,strlen(buf));
		if(rv < 0)
		{
			zlog_error(zc,"write to server failure:%s",strerror(errno));
			goto cleanup;
		}
		zlog_info(zc,"write to server successfully");

		rv = read(conn_fd,buf,sizeof(buf));
		if(rv < 0)
		{
			zlog_error(zc,"read data from server failure:%s",strerror(errno));
			goto cleanup;
		}
		else if(rv == 0)
		{
			zlog_error(zc,"socket[%d] get disconnected",conn_fd);
			goto cleanup;
		}
		else if(rv > 0)
		{
			zlog_info(zc,"read %d bytes data from server",rv);
		}
		sql_select();
		sql_delect();
		/* get the diff time */
		while(1)
		{
			get_second_time(&t_end);
			diff_time = t_end - t_begin;
			if(diff_time == tim)
			{
				break;
			}
			else
			{
				continue;
			}
		}

		sql_delect();
		zlog_fini();
		printf("\n");
	}
cleanup:
	close(conn_fd);
	return 0;
}

void printf_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("--h(--hostname): sepcify server hostname \n");
	printf("-p(--port): sepcify server port \n");
	printf("-t(--tim):get the temperature time \n");
	printf("-H(--Help): print this help information \n");
}
/* get the temp */
int get_temp(float *temp,char *chip)
{
	int     rv = 0;
	int     fd = -1;
	int     found = 0;
	char    *ptr = 0;
	char    buf[1024];
	char    ds_path[50];
	char    *w1_path = "/sys/bus/w1/devices";
	DIR     *dirp = 0;
	struct  dirent  *direntp;

	if((dirp = opendir(w1_path)) == NULL )
	{
		printf("opendir error: %s\n",strerror(errno));
		return -1;
	}

	while((direntp = readdir(dirp)) != NULL)
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strcpy(chip,direntp->d_name);
			found = 1;
			break;
		}
	}
	closedir(dirp);
	
	if(!found)
	{
		printf("can not find ds18b20 in %s\n",w1_path);
		return -1;
	}

	snprintf(ds_path,sizeof(ds_path),"%s/%s/w1_slave",w1_path,chip);
	if((fd = open(ds_path,O_RDONLY)) < 0 )
	{
		printf("open %s error : %s\n",ds_path,strerror(errno));
		return -1;
	}
	if(read(fd,buf,sizeof(buf)) < 0)
	{
		printf("read %s error:%s\n",w1_path,strerror(errno));
		rv = -1;
		goto cleanup;
	}

	ptr = strstr(buf,"t=");
	if(!ptr)
	{
		printf("error:can not get temperature\n");
		rv = -1;
		goto cleanup;
	}

	ptr += 2;
	*temp = atof(ptr)/1000;
	snprintf(buf,sizeof(buf),"%f",*temp);

cleanup:
	close(fd);
	return rv;
}

int get_first_time(long int *t_begin)
{
	time_t time_first;
	time(&time_first);
	*t_begin = time_first;
	return 0;
}

int get_second_time(long int *t_end)
{
	time_t time_second;
	time(&time_second);
	*t_end = time_second;
	return 0;
}

sqlite3 *sql_conn(char *db_name)
{
	int ret;
	ret = sqlite3_open(db_name,&db);
	if(ret != SQLITE_OK)
	{
		printf("sql connect error:%s\n",sqlite3_errmsg(db));
	}
	printf("sql connect successfully\n");
	return db;
}

void sql_create()
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	sprintf(sql,"create table if not exists data(""time char(50),"
												"temp char(50),"
												"chip char(20));");
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		printf("sql create table failure:%s\n",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);

	}
	printf("create table successfully\n");
}

void sql_insert(char *begin_time,char s_temp[50],char *chip)
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	sprintf(sql,"insert into data values('time: %s','temp: %s','chip: %s');",begin_time,s_temp,chip);
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		printf("insert error:%s\n",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);
	}
	printf("insert into table successfully\n");
}

void sql_select()
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from data;");
	ret = sqlite3_exec(db,sql,callback,NULL,&error);
	if(ret != SQLITE_OK)
	{
		printf("select failure:%s\n",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);
	}
	printf("\n------------------------------");
}

void sql_delect()
{
	int ret;
	char *error = 0;
	char sql[maxn] = {0};

	memset(sql,0,sizeof(sql));
	sprintf(sql,"delete from data;");
	ret = sqlite3_exec(db,sql,NULL,NULL,&error);
	if(ret != SQLITE_OK)
	{
		printf("delete table error:%s\n",error);
		sqlite3_close(db);
		sqlite3_free(error);
		exit(1);
	}
}

void sql_close()
{
	int ret;
	char sql[maxn] = {0};

	ret = sqlite3_close(db);
	if(ret != SQLITE_OK)
	{
		printf("close sql error:%s\n",strerror(errno));
	}
}

int callback(void *arg,int column,char **value,char **name)
{
	int i;
	for(i=0; i<column; i++)
	{
		printf("%s",value[i]?value[i]:"NULL");
	}
	return 0;
}

/* sigal handle */
void sig_handle(int signum)
{
	if(SIGINT == signum)
	{
		printf("SIGINT signal detected\n");
		g_stop = 1;
	}
}

