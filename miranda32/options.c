/*
Miranda ICQ: the free icq client for MS Windows 
Copyright (C) 2000  Roland Rabien

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

For more information, e-mail figbug@users.sourceforge.net
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "Encryption.h"

// global variables
extern HINSTANCE				ghInstance;
extern HWND						ghwnd;
extern OPTIONS					opts;
extern OPTIONS_RT				rto;

extern char						mydir[MAX_PATH];

char tmpfn[MAX_PATH]; //temp to hold full path for ini file

//defines
#define T_STR		0
#define T_INT		1

// types
typedef struct 
{
	enum OPT_IDX idx;
	char *name;
	int type;	
	unsigned int defInt;
	char *defChar;
} REG_OPT;

void LoadFromReg(HKEY hkey, char *postkey, enum OPT_IDX idx, void *data);
void SaveToReg(HKEY hkey, char *postkey, enum OPT_IDX idx, void *data);
HKEY GetUserRegKey(char *postkey);
HKEY GetAppRegKey(void);

void GetMirandaInt(int *val, HKEY hkey, char *name, int def);
void GetMirandaStr(char *val, HKEY hkey, char *name, char *def);
void SetMirandaInt(int val, HKEY hkey, char *name);
void SetMirandaStr(char *val, HKEY hkey, char *name);

void LoadOptions(OPTIONS *opts, char *postkey)
{
	HKEY hkey;
	HKEY hkeyapp;

	char tmp[16];
	
	if (!rto.useini)
	{
		if (!postkey) postkey = "";
		hkey = GetUserRegKey(postkey);
		hkeyapp = GetAppRegKey();
	}

	strcpy(tmpfn, rto.inifile);
	//main window pos
	GetMirandaInt(&opts->pos_mainwnd.xpos, hkey, "xpos", 50);
	GetMirandaInt(&opts->pos_mainwnd.ypos, hkey, "ypos", 50);
	GetMirandaInt(&opts->pos_mainwnd.width, hkey, "width", 150);
	GetMirandaInt(&opts->pos_mainwnd.height, hkey, "height", 260);
	//send msg window pos
	GetMirandaInt(&opts->pos_sendmsg.xpos, hkey, "xpos_sendmsg", -1);
	GetMirandaInt(&opts->pos_sendmsg.ypos, hkey, "ypos_sendmsg", -1);
	GetMirandaInt(&opts->pos_sendmsg.width, hkey, "width_sendmsg", -1);
	GetMirandaInt(&opts->pos_sendmsg.height, hkey, "height_sendmsg", -1);
	//send url window pos
	GetMirandaInt(&opts->pos_sendurl.xpos, hkey, "xpos_sendurl", -1);
	GetMirandaInt(&opts->pos_sendurl.ypos, hkey, "ypos_sendurl", -1);
	GetMirandaInt(&opts->pos_sendurl.width, hkey, "width_sendurl", -1);
	GetMirandaInt(&opts->pos_sendurl.height, hkey, "height_sendurl", -1);
	//recv msg window pos
	GetMirandaInt(&opts->pos_recvmsg.xpos, hkey, "xpos_recvmsg", -1);
	GetMirandaInt(&opts->pos_recvmsg.ypos, hkey, "ypos_recvmsg", -1);
	GetMirandaInt(&opts->pos_recvmsg.width, hkey, "width_recvmsg", -1);
	GetMirandaInt(&opts->pos_recvmsg.height, hkey, "height_recvmsg", -1);
	//recv url window pos
	GetMirandaInt(&opts->pos_recvurl.xpos, hkey, "xpos_recvurl", -1);
	GetMirandaInt(&opts->pos_recvurl.ypos, hkey, "ypos_recvurl", -1);
	GetMirandaInt(&opts->pos_recvurl.width, hkey, "width_recvurl", -1);
	GetMirandaInt(&opts->pos_recvurl.height, hkey, "height_recvurl", -1);

	GetMirandaInt(&opts->uin, hkey, "uin", 0);
	GetMirandaInt(&opts->port, hkey, "port", 4000);
	GetMirandaInt(&opts->flags1, hkey, "flags_1", 0);
	GetMirandaInt(&opts->alpha, hkey, "alpha", 200);
	GetMirandaInt(&opts->autoalpha, hkey, "autoalpha", 150);
	GetMirandaInt(&opts->playsounds, hkey, "sounds", 1);
	GetMirandaInt(&opts->defaultmode, hkey, "defmode", -1);
	GetMirandaInt(&opts->hotkey_showhide, hkey, "hotkey_showhide", 'A');
	GetMirandaInt(&opts->hotkey_setfocus, hkey, "hotkey_setfocus", 'I');
	GetMirandaInt(&opts->hotkey_netsearch, hkey, "hotkey_netsearch", 'S');

	GetMirandaStr(tmp, hkey, "passwordenc", "");
	if (tmp[0]==0)
	{ //no enc password there -check for a non encryped password
		GetMirandaStr(opts->password, hkey, "password", "mc_hammer");
	}
	else
	{
		Encrypt(tmp,FALSE);
		strcpy(opts->password,tmp);
	}
	

	GetMirandaStr(opts->hostname, hkey, "hostname", "icq.mirabilis.com");

	
	GetMirandaStr(opts->contactlist, hkey, "contactlist", "default.cnt");

	
	
	GetMirandaStr(opts->history, hkey, "history", "default.hst");

	GetMirandaInt(&opts->proxyuse, hkey, "proxyuse", 0);
	GetMirandaStr(opts->proxyhost, hkey, "proxyhost", "localhost");
	GetMirandaInt(&opts->proxyport, hkey, "proxyport", 1234);
	GetMirandaInt(&opts->proxyauth, hkey, "proxyauth", 0);
	GetMirandaStr(opts->proxyname, hkey, "proxyname", "username");
	GetMirandaStr(opts->proxypass, hkey, "proxypass", "password");
	
	if (!rto.useini)
	{	
		RegCloseKey(hkey);
		RegCloseKey(hkeyapp);
	}
}

void SaveOptions(OPTIONS *opts, char *postkey)
{
	HKEY hkey;
	HKEY hkeyapp;
	
	char *h;
	char *c;

	if (!rto.useini)
	{
		if (!postkey) postkey = "";
		hkey = GetUserRegKey(postkey);
		hkeyapp = GetAppRegKey();
	}

	strcpy(tmpfn, rto.inifile);
	//main window pso
	SetMirandaInt(opts->pos_mainwnd.xpos, hkey, "xpos");
	SetMirandaInt(opts->pos_mainwnd.ypos, hkey, "ypos");
	SetMirandaInt(opts->pos_mainwnd.width, hkey, "width");
	SetMirandaInt(opts->pos_mainwnd.height, hkey, "height");
	//send msg window pos
	SetMirandaInt(opts->pos_sendmsg.xpos, hkey, "xpos_sendmsg");
	SetMirandaInt(opts->pos_sendmsg.ypos, hkey, "ypos_sendmsg");
	SetMirandaInt(opts->pos_sendmsg.width, hkey, "width_sendmsg");
	SetMirandaInt(opts->pos_sendmsg.height, hkey, "height_sendmsg");
	//send URL window pos
	SetMirandaInt(opts->pos_sendurl.xpos, hkey, "xpos_sendurl");
	SetMirandaInt(opts->pos_sendurl.ypos, hkey, "ypos_sendurl");
	SetMirandaInt(opts->pos_sendurl.width, hkey, "width_sendurl");
	SetMirandaInt(opts->pos_sendurl.height, hkey, "height_sendurl");
	//recv msg window pos
	SetMirandaInt(opts->pos_recvmsg.xpos, hkey, "xpos_recvmsg");
	SetMirandaInt(opts->pos_recvmsg.ypos, hkey, "ypos_recvmsg");
	SetMirandaInt(opts->pos_recvmsg.width, hkey, "width_recvmsg");
	SetMirandaInt(opts->pos_recvmsg.height, hkey, "height_recvmsg");
	//recv URL window pos
	SetMirandaInt(opts->pos_recvurl.xpos, hkey, "xpos_recvurl");
	SetMirandaInt(opts->pos_recvurl.ypos, hkey, "ypos_recvurl");
	SetMirandaInt(opts->pos_recvurl.width, hkey, "width_recvurl");
	SetMirandaInt(opts->pos_recvurl.height, hkey, "height_recvurl");

	SetMirandaInt(opts->uin, hkey, "uin");
	SetMirandaInt(opts->port, hkey, "port");
	SetMirandaInt(opts->flags1, hkey, "flags_1");
	SetMirandaInt(opts->alpha, hkey, "alpha");
	SetMirandaInt(opts->autoalpha, hkey, "autoalpha");
	SetMirandaInt(opts->playsounds, hkey, "sounds");
	SetMirandaInt(opts->defaultmode, hkey, "defmode");
	SetMirandaInt(opts->hotkey_showhide, hkey, "hotkey_showhide");
	SetMirandaInt(opts->hotkey_setfocus, hkey, "hotkey_setfocus");
	SetMirandaInt(opts->hotkey_netsearch, hkey, "hotkey_netsearch");

	//NO LONGER save it as a non-encryped password, only as a encytped password
	//delte the "password" key
	RegDeleteValue(hkey,"password");

	Encrypt(opts->password,TRUE);
	SetMirandaStr(opts->password, hkey, "passwordenc");
	Encrypt(opts->password,FALSE);

	c = strrchr(opts->contactlist, '\\');
	if (c)
		c++;
	else
		c = opts->contactlist;

	h = strrchr(opts->history, '\\');
	if (h)
		h++;
	else
		h = opts->history;

	SetMirandaStr(opts->hostname, hkey, "hostname");
	SetMirandaStr(c, hkey, "contactlist");
	SetMirandaStr(h, hkey, "history");

	SetMirandaInt(opts->proxyuse, hkey, "proxyuse");
	SetMirandaStr(opts->proxyhost, hkey, "proxyhost");
	SetMirandaInt(opts->proxyport, hkey, "proxyport");
	SetMirandaInt(opts->proxyauth, hkey, "proxyauth");
	SetMirandaStr(opts->proxyname, hkey, "proxyname");
	SetMirandaStr(opts->proxypass, hkey, "proxypass");

	if (!rto.useini)
	{
		RegCloseKey(hkey);
		RegCloseKey(hkeyapp);
	}
}

void GetMirandaInt(int *val, HKEY hkey, char *name, int def)
{	
	UINT ret;
	if (rto.useini)
	{
		
		ret=GetPrivateProfileInt("Miranda ICQ", name, def, tmpfn);
		def=ret;
		*val=def;
	}
	else
	{
		long sz;
		int res;

		if (hkey)
		{
			sz = sizeof(long);
			res = RegQueryValueEx(hkey, name, 0, NULL, (BYTE *)val, &sz);
		}
		if (hkey == NULL || res != ERROR_SUCCESS)
		{
			*val = def;
		}

	}
}

void GetMirandaStr(char *val, HKEY hkey, char *name, char *def)
{
	if (rto.useini)
	{

		GetPrivateProfileString("Miranda ICQ", name, def, val, 128, tmpfn);
	}
	else
	{
		long sz;
		int res;

		if (hkey)
		{
			sz = 128;
			res = RegQueryValueEx(hkey, name, 0, NULL, val, &sz);
		}
		if (hkey == NULL || res != ERROR_SUCCESS)
		{
			strcpy(val, def);
		}
	}
}

void SetMirandaInt(int val, HKEY hkey, char *name)
{
	if (rto.useini)
	{
		char buf[64];
		sprintf(buf, "%d", val);
		WritePrivateProfileString("Miranda ICQ", name, buf, tmpfn);
	}
	else
	{
		long sz;
		if (hkey)
		{
			sz = sizeof(long);
			RegSetValueEx(hkey, name, 0, REG_DWORD, (BYTE *)&val, sz);
		}
	}
}

void SetMirandaStr(char *val, HKEY hkey, char *name)
{
	if (rto.useini)
	{
		WritePrivateProfileString("Miranda ICQ", name, val, tmpfn);
	}
	else
	{
		long sz;
		if (hkey)
		{
			sz = strlen(val)+1;
			RegSetValueEx(hkey, name, 0, REG_SZ, val, sz);
		}
	}
}

HKEY GetAppRegKey(void)
{
	HKEY hkey;

	char regkey[128] = "Software\\FigBug Developments\\Miranda ICQ";

	if (RegCreateKey(HKEY_CURRENT_USER, regkey, &hkey) == ERROR_SUCCESS)
	{
		return hkey;
	}
	else
	{
		return NULL;
	}	
}

HKEY GetUserRegKey(char *postkey)
{
	HKEY hkey;

	char regkey[128] = "Software\\FigBug Developments\\Miranda ICQ\\";

	if (!strcmp(postkey, ""))
	{
		strcat(regkey, "default");
	}
	else
	{
		strncat(regkey, postkey, 12);
		strcat(regkey, "");
	}

	if (RegCreateKey(HKEY_CURRENT_USER, regkey, &hkey) == ERROR_SUCCESS)
	{
		return hkey;
	}
	else
	{
		return NULL;
	}	
}