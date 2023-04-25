/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_temp.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/18/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/18/23 09:59:39"
 *                 
 ********************************************************************************/
#include "get_temp.h"
#include "zlog.h"

int get_temp(float *temp)
{
	int rv = 0;
	int fd = -1; 
	int found = 0;
	char *ptr = 0;
	char chip[40];
	char buf[1024];
	char ds_path[50];
	char *w1_path = "/sys/bus/w1/devices";

	DIR *dirp = 0;
	struct dirent *direntp;

	if((dirp = opendir(w1_path)) == NULL )
	{   
		rv = -1;
	//	zlog_error(zc,"opendir error: %s",strerror(errno));
		goto cleanup; 
	}   

	while((direntp = readdir(dirp)) != NULL)
	{   
		if(strstr(direntp->d_name,"28-"))
		{   
			memset(chip,0,sizeof(chip));
			strcpy(chip,direntp->d_name);
			found = 1;
			break;
		}
	}

	if(!found)
	{
		rv = -2;
	//	zlog_error(zc,"can not find ds18b20 in %s",w1_path);
		goto cleanup;
	}
	closedir(dirp);

	snprintf(ds_path,sizeof(ds_path),"%s/%s/w1_slave",w1_path,chip);
	if((fd = open(ds_path,O_RDONLY)) < 0 )
	{
		rv = -3;
	//	zlog_error(zc,"open %s error : %s",ds_path,strerror(errno));
		goto cleanup;
	}

	memset(buf,0,sizeof(buf));
	if(read(fd,buf,sizeof(buf)) < 0)
	{
		rv = -4;
	//	zlog_error(zc,"read %s error:%s",w1_path,strerror(errno));
		goto cleanup;
	}

	ptr = strstr(buf,"t=");
	if(!ptr)
	{
		rv = -5;
	//	zlog_error(zc,"get temperature failure");
		goto cleanup;
	}

	ptr += 2;
	*temp = atof(ptr)/1000;
	snprintf(buf,sizeof(buf),"%f",*temp);

cleanup:
	if(rv < 0)
	{
		close(fd);
		return rv;
	}
	else
	{
		return rv;
	}
}
