/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  zlog.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/04/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/04/23 14:27:14"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include "zlog.h"

int main(int argc,char *argv[])
{
	int rc;
	rc = zlog_init("zlog.conf");
	if (rc) 
	{
		printf("init failed\n");
		return -1;
	}

	zlog_info("hello, zlog info"); 

	zlog_error("hello, zlog error");

	zlog_warn("hello, zlog warning"); 

	zlog_debug("hello, zlog debug"); 

	zlog_fini();  
	return 0;
}
