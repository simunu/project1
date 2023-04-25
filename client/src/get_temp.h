/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_temp.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/17/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/17/23 15:21:02"
 *                 
 ********************************************************************************/
#ifndef __GETTEMP_H_
#define __GETTEMP_H_
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

/* get the temperature */
extern int get_temp(float *temp);
#endif	/* ----- #ifndef _GETEMP_H_ ----- */
