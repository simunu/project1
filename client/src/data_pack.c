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
	time_t timep;
	struct tm *ptm;
	int tim = 0;

	time(&timep);
	ptm = localtime(&timep);
	if(ptm != NULL)
	{
		snprintf(buff,len,"%04d-%02d-%2d %02d:%02d:%02d",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
		tim = timep;
		return tim;
	}
	
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
	snprintf(data->sn,sizeof(data->sn),"%s",devsn);

	rv = get_temp(&temp);
	if(rv < 0)
	{
		return rv;
	}
	snprintf(data->s_temp,sizeof(data->s_temp),"%f",temp);

	*tim_first = get_time(time,sizeof(time));
	if(*tim_first < 0)
	{
		return -1;
	}
	snprintf(data->time,sizeof(data->time),"%s",time);
	return 0;
}
