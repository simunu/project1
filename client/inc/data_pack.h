/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  data_pack.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/18/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/18/23 15:44:24"
 *                 
 ********************************************************************************/
#ifndef __DATA_PACK_H_
#define __DATA_PACK_H_
#include <stdio.h>
#include <string.h>
struct send_data
{
	char s_temp[40];
	char time[40];
	char sn[40];
};
int get_temp(float *temp);
int get_time(char *buff,int len);
int data_pack(struct send_data* data,char *devsn,int *tim_first);
#endif /* ----- #ifndef __DATA_PACK_H_ ----- */
