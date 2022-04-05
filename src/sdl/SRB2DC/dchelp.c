// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004 by Sonic Team Jr.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      stub and replacement "ANSI" C functions for use on Dreamcast/KOS
// 
//-----------------------------------------------------------------------------
#include <kos/fs.h>
#include "../../doomdef.h"
#include "dchelp.h"

int access(const char* path, int amode)
{
	file_t handle = FILEHND_INVALID;

	if(amode == F_OK || amode == R_OK)
		handle=fs_open(path,O_RDONLY);
	else if(amode == (R_OK|W_OK))
		handle=fs_open(path,O_RDWR);
	else if(amode == W_OK)
		handle=fs_open(path,O_WRONLY);

	if(handle != FILEHND_INVALID)
	{
		fs_close(handle);
		return 0;
	}

	return -1;
}

int mkdir(const char *_path, mode_t __mode )
{
	__mode = 0;
	return fs_mkdir(_path);
}

char *getcwd(char *__buf, size_t __size )
{
	return strncpy(__buf,fs_getwd(),__size);
}

int chdir(const char *folder)
{
	return fs_chdir(folder);
}

#if !(defined(NONET) || defined(NOMD5))
#include <lwip/lwip.h>

static uint8 ip[4];
static char *h_addr_listtmp[2] = {ip, NULL};
static struct hostent hostenttmp = {NULL, NULL, 0, 1, h_addr_listtmp};

struct hostent *gethostbyname(const char *name)
{
	struct sockaddr_in dnssrv;
	dnssrv.sin_family = AF_INET;
	dnssrv.sin_port = htons(53);
	dnssrv.sin_addr.s_addr = htonl(0x0a030202); ///< what?
	if(lwip_gethostbyname(&dnssrv, name, ip) < 0)
		return NULL;
	else
		return &hostenttmp;
}
#endif
