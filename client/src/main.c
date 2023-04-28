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
	int diff_time = 0;
	int tim_first = 0;
	int con_flag = 1;
	int sample_flag = 0;
	char buf[1024];
	char str[1024];
	char s_temp[64];
	char buf_send[1024];
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
		con_flag = 0;
		zlog_error(zc,"socket connect failure");
		socket_close(&sock);
		return -1;
	}
	else
	{
		con_flag = 1;
		zlog_info(zc,"socket connnect successfully");
	}

	while(!g_stop)
	{
		/* open database and create table */
		db = open_database(db_name);
		if(!db)
		{   
			db = open_database(db_name);
		}   

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

		/* judge socket state */
		if((tcp_state(&sock)) < 0)
		{
			con_flag = 0;
			socket_close(&sock);
			if((socket_conn(&sock,hostname,port)) < 0)
			{
				zlog_warn(zc,"socket reconnect failure");
			}
			else
			{
				con_flag = 1;
				zlog_info(zc,"socket reconnect successfully");
			}
		}
		else
		{
			con_flag = 1;
		}

		/* when socket connect,send pack data */
		if(con_flag)
		{
			if(sample_flag)
			{
				memset(buf,0,sizeof(buf));
				snprintf(buf,sizeof(buf),"time: %s|temp: %s|sn: %s",data.time,data.s_temp,data.sn);
				printf("send data-- %s\n",buf);
				rv = socket_send(&sock,buf,sizeof(buf));
				if(rv < 0)
				{
					sql_insert(&data);
					zlog_error(zc,"send data to server failure");
					socket_close(&sock);
				}
				else
				{
					zlog_info(zc,"send data to server successfully");
				}
				
				/* send  data about disconnect insert into database  */
				rv = sql_data_count();
				if( rv > 0 )
				{
					sql_get_data(buf_send,sizeof(buf_send),sql_data_count());
					printf("database data-- %s\n",buf_send);
					rv = write(sock.conn_fd,buf_send,sizeof(buf_send));
					if(rv < 0 )
					{
						zlog_error(zc,"send data to server from database failure");
						socket_close(&sock);
					}
					else
					{
						sql_delete();
						zlog_info(zc,"send data to server from database successfully and delete them");
					}

				}
				else
				{
					zlog_info(zc,"database not have data");
				}		
			}
		}

		if(!con_flag)
		{
			if(sample_flag)
			{
				zlog_info(zc,"%s %s %s\n",data.time,data.s_temp,data.sn);
				sql_insert(&data);
			}
			else
			{
				zlog_info(zc,"no data to do");
			}
		}
		printf("--------------------\n");

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
	sql_close();
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

