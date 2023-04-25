/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  data_pack.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/18/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/18/23 15:38:52"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include "data_pack.h"
#include "zlog.h"

int get_time(char *buff,int len)
{
	struct timeval tv;
	struct tm* ptm;
	char time_string[50];
	int tim = 0;
	int rv = 0;

	gettimeofday(&tv, NULL);
	ptm = localtime (&(tv.tv_sec));
	if(ptm != NULL)
	{
		tim = tv.tv_sec;
		strftime(time_string,sizeof(time_string),"%Y-%m-%d %H:%M:%S",ptm);
		snprintf(buff,len,"%s",time_string);
		rv = tim;
	}
	else
	{
		zlog_error(zc,"error: invalid time pointer");
		rv = -1;
	}
	return rv;
}

int data_pack(s_data *data,char *devsn,int *tim_first)
{
	int rv = 0;
	float temp = 0;
	char time[40];

	if(!devsn )
	{
		zlog_error(zc,"invalid input arguments");
		return -1;
	}

	sprintf(data->sn,"%s",devsn);

	rv = get_temp(&temp);
	if(rv < 0)
	{
		return rv;
	}
	sprintf(data->s_temp,"%f",temp);

	*tim_first = get_time(time,sizeof(time));
	if(*tim_first < 0)
	{
		return -1;
	}
	sprintf(data->time,"%s",time);
	return 0;
}
