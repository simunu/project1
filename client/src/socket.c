/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/18/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/18/23 10:42:45"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include "socket.h"
#include "zlog.h"

int socket_conn(sock_t *sock,char *hostname,int port)
{
	int rv = 0;
	int conn_fd = -1;
	char ipstr[1024];
	char **hostip = 0;
	struct sockaddr_in servaddr;
	struct hostent *servhost;
	
	if(!hostname || !port)
	{
		zlog_error(zc,"invalid input arguments");
	}

	sock->port = port;
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
				snprintf(sock->servip,sizeof(sock->servip),"%s",ipstr);
				break;
			default:
				printf("error address\n");
				break;
		}   
	} 
	else
	{
		snprintf(sock->servip,sizeof(sock->servip),"%s",hostname);
	}

	sock->conn_fd = socket(AF_INET,SOCK_STREAM,0);
	if(sock->conn_fd < 0)
	{
		zlog_warn(zc,"connect to server failure:%s",strerror(errno));
		socket_close(sock->conn_fd);
		return -2;
	}

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(sock->port);
	inet_aton(sock->servip,&servaddr.sin_addr);

	if((connect(sock->conn_fd,(struct sockaddr *)&servaddr,sizeof(servaddr))) < 0)
	{
		socket_close(sock->conn_fd);
		return -3;
	}
	
	return sock->conn_fd;
	
}

int tcp_state(sock_t *sock)
{
	struct tcp_info info;
	int len = sizeof(info);

	if(sock->conn_fd < 0)
	{
		socket_close(sock->conn_fd);
		return -1;
	}

	getsockopt(sock->conn_fd,IPPROTO_TCP,TCP_INFO,&info,(socklen_t *)&len);
	if((info.tcpi_state != TCP_ESTABLISHED))
	{
		socket_close(sock->conn_fd);
		return -1;
	}
	else
	{
		return 0;
	}
}

int socket_write(sock_t *sock,s_data *data)
{
	int rv = 0;
	char buf[1024];
	
	if(sock->conn_fd < 0)
	{
		socket_close(sock->conn_fd);
		return -1;
	}

	memset(buf,0,sizeof(buf));
	snprintf(buf,sizeof(buf),"time: %s\ntemp: %s\nsn: %s\n",data->time,data->s_temp,data->sn);

	rv = write(sock->conn_fd,buf,strlen(buf));
	if(rv < 0)
	{
		zlog_error(zc,"write to server failure:%s",strerror(errno));
		socket_close(sock->conn_fd);
		return -2;
	}
	zlog_info(zc,"write to server successfully");

	rv = read(sock->conn_fd,buf,strlen(buf));
	if(rv < 0)
	{
			zlog_warn(zc,"read data from server failure:%s",strerror(errno));
		socket_close(sock->conn_fd);
		return -3;
	}
	else if(rv == 0)
	{
			zlog_warn(zc,"socket[%d] get disconnected",sock->conn_fd);
		socket_close(sock->conn_fd);
		return -4;
	}
	else
	{
		return 0;
	}
}

int socket_close(int conn_fd)
{
	close(conn_fd);
	conn_fd = -1;
}
