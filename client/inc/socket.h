/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/17/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/17/23 13:55:10"
 *                 
 ********************************************************************************/
#ifndef __SOCKET_H_
#define __SOCKET_H_
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <netdb.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

struct socket_data
{
	int conn_fd;
	int port;
	char servip[40];
};
void printf_usage(char *progname);
int socket_init(struct socket_data* sock,char *hostname,int port);
int socket_conn(struct socket_data* sock);
int tcp_state(struct socket_data* sock);
int socket_w_r(struct socket_data* sock,char *buf);
#endif /* ----- #ifdef __SOCKET_H_ ----- */
