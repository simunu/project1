/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/18/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/18/23 10:04:50"
 *                 
 ********************************************************************************/
#include "main.h"

static int g_stop = 0;

int main(int argc,char **argv)
{
	int ch = 0;
	int tim = 0;
	int rv = 0;
	int port = 8808;
	int sample_flag = 0;
	int diff_time = 0;
	int tim_first = 0;
	char buf[1024];
	char str[1024];
	char s_temp[50];
	char end_time[40];
	char *servip = 0;
	char *devsn = "rpi1002";
	char *hostname = "127.0.0.1";
	char *db_name = "client1.db";
	time_t tloc;

	sock_t sock;
	s_data data;
	struct sockaddr_in servaddr;

	struct option opts[] = {
		{"hostname",required_argument,NULL,'h'},
		{"port",required_argument,NULL,'p'},
		{"time interval",required_argument,NULL,'t'},
		{"devsn",required_argument,NULL,'s'},
		{"Help",no_argument,NULL,'H'},
		{NULL,0,NULL,0}
	};

	while((ch = getopt_long(argc,argv,"h:p:t:s:H",opts,NULL)) != -1)
	{
		switch(ch)
		{
			case'h':
				hostname = optarg;
				break;
			case'p':
				port = atoi(optarg);
				break;
			case 't':
				tim = atoi(optarg);
				break;
			case 's':
				devsn = optarg;
				break;
			case'H':
				printf_usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	signal(SIGINT,sig_handle);

	/* create zlog for client*/
	if(!(get_zlog()))
	{
		printf("get zlog failure\n");
	}
	else
	{
		printf("get zlog successfully\n");
	}

	/* connect to socket for the first time */
	if((socket_conn(&sock,hostname,port)) < 0)
	{
		zlog_error(zc,"socket connect failure");
		socket_close(sock.conn_fd);
	}
	else
	{
		zlog_info(zc,"socket connnect successfully");
	}

	db = open_database(db_name);
	if(!db)
	{
		db = open_database(db_name);
	}
	
	while(!g_stop)
	{
		sample_flag = 0;

		/* get data and pack them */
		rv = data_pack(&data,devsn,&tim_first);
		if(rv < 0)
		{
			zlog_error(zc,"get data failure");
		}
		else
		{
			zlog_info(zc,"pack data successfully");
			sample_flag = 1;
		}

		/* judge socket state,if disconnect connect again */
		rv = tcp_state(&sock);
		if(rv < 0)
		{
			zlog_info(zc,"clinet wait for connecting.......");
			while(1)
			{
				rv = socket_conn(&sock,hostname,port);
				if(rv < 0)
				{
					continue;
				}
				else
				{
					zlog_info(zc,"connnect to server successfully again");
					break;
				}
			}
		}

		/* pack data and connect server successfully,begin to send data */
		if(sample_flag == 1)
		{
			rv = sql_insert(&data);
			if(rv < 0)
			{
				zlog_error(zc,"insert data to database failure");
			}
			else
			{
				zlog_info(zc,"insert data to database successfully");
			}

			/* while connect to server successfully,send data to server */
			rv = socket_write(&sock,&data);
			if(rv < 0)
			{
				zlog_error(zc,"send data to server failure");
			}
			else
			{
				zlog_info(zc,"send data to server successfully");
			}
		}

		rv = sql_select();
		if(rv < 0)
		{
			zlog_error(zc,"select data from database failure");
		}
		sql_delect();
		
		/* wait for time become the input time interval*/
		while(1)
		{
			time(&tloc);
			diff_time = tloc - tim_first;
			if(diff_time >= tim)
			{
				break;
			}
			else
			{
				continue;
			}
		}
	}
	close(sock.conn_fd);
	zlog_fini();
	return 0;
}

void printf_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("--h(--hostname): sepcify server hostname \n");
	printf("-p(--port): sepcify server port \n");
	printf("-s(--devsn): the serial number of the device \n");
	printf("-t(--tim): get the temperature time \n");
	printf("-H(--Help): print this help information \n");
}

void sig_handle(int signum)
{
	if(SIGINT == signum)
	{   
		printf("SIGINT signal detected\n");
		g_stop = 1;
	}   
}

