/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_zlog.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/18/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/18/23 10:54:09"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "zlog.h"

zlog_category_t *get_zlog(void)
{
	if((zlog_init("cli_zlog.ini")))
	{
		printf("zlog init failure\n");
		exit(1);
	}

	zc = zlog_get_category("m_cli");
	if(!zc)
	{
	//	printf("zlog get category failure\n");
		zlog_fini();
		exit(1);
	}
//	zlog_info(zc,"zlog get category successfully");
	return zc;
}


