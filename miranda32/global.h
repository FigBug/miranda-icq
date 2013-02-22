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

#ifndef _global_h
#define _global_h

#include <windows.h>
#include <commctrl.h>

//If u dont have the Win2k SDK
#ifdef _NOSDK //in ops, get rid of _NOSDK to ignore the contents of this ifdef
	#define LWA_ALPHA 0x02
	#define WS_EX_LAYERED 0x080000
#endif


#define MIRANDABOT		0L
#define MIRANDAVERSTR   "0.0.6"
#define VERSIONSTR		"http://miranda-icq.sourceforge.net/cgi-bin/version?0.0.6"
#define HELPSTR			"http://miranda-icq.sourceforge.net/help"
#define MIRANDANAME		"Miranda"
#define MIRANDAWEBSITE  "http://Miranda-icq.SourceForge.net/"


// defines
#define TM_CHECKDATA	0x01
#define TM_KEEPALIVE	0x02
#define TM_OTHER		0x03
#define TM_ONCE			0x04
#define TM_TIMEOUT		0x05
#define TIMEOUT_TIMEOUT 20000 //timeou in mili seconds
#define TM_AUTOALPHA	0x06
#define TM_MSGFLASH		0x07
#define TIMEOUT_MSGFLASH 500

//hotkey defines
#define HOTKEY_SHOWHIDE 0x01
#define HOTKEY_SETFOCUS 0x02
#define HOTKEY_NETSEARCH 0x04

//history defines - type of history event (passed to AddToHistory)
#define HISTORY_MSGRECV 1
#define HISTORY_MSGSEND 2
#define HISTORY_URLRECV 3
#define HISTORY_URLSEND 4

#define WT_RECVMSG		0x01
#define WT_SENDMSG		0x02
#define WT_SENDURL		0x03
#define WT_ADDED		0x04
#define WT_RECVURL		0x05
#define WT_AUTHREQ		0x06
#define WT_ADDCONT		0x07
#define WT_RESULT		0x08
#define WT_DETAILS		0x09
#define WT_RECVFILE		0x10

#define MAX_CONTACT		1024

#define TI_CREATE		0
#define TI_DESTROY		1
#define TI_UPDATE		2

#define ET_AMES			0x0001
#define ET_RIH			0x0002
#define ET_RINH			0x0004
#define ET_ANET			0x0008
#define ET_USTAT		0x0010
#define ET_PWM			0x0020

// message types
#define TI_CALLBACK		WM_USER + 4
#define TI_USERFOUND	WM_USER + 5
#define TI_INFOREPLY	WM_USER + 6
#define TI_EXTINFOREPLY WM_USER + 7
#define TI_SRVACK		WM_USER + 8
#define TI_SORT			WM_USER + 9
#define TI_SHOW			WM_USER + 10

// some kick ass o rama flags
#define FG_MIN2TRAY		0x01
#define FG_ONTOP		0x02
#define FG_POPUP		0x04
#define FG_TRANSP		0x08
#define FG_TOOLWND		0x10
#define FG_ONECLK		0x20

#define UF_IGNORE		0x01
#define UF_AUTOCHAT		0x02

#define MAX_PROFILESIZE 30
// global types

typedef struct {
	unsigned long uin;
	const char *msg;
	const char *url;
	const char *descr;
	const char *nick;
	const char *email;
	const char *first;
	const char *last;
	char auth;
	unsigned long seq;
	// the day
	unsigned char hour;
	unsigned char minute;
	unsigned char day;
	unsigned char month;
	unsigned short year;

	int msgid; //id to the src msgque array
} DLG_DATA;

typedef struct WNODE HWNDLIST;
typedef struct WNODE {
	HWND hwnd;
	HWNDLIST *next;
	int windowtype;
	unsigned int uin;
	
	unsigned int ack;
} HWNDNODE;

typedef struct {
	HWND				hwndStatus;
	HWND				hwndContact;
	HWND				hwndDlg;
	HIMAGELIST			himlIcon;
	BOOL				online;
	BOOL				net_activity;
	HWNDLIST			*hwndlist;
	unsigned long		status;
	BOOL				hidden;
	int					sortmode;
	int					askdisconnect;
	int					useini;
	char				inifile[MAX_PATH];
} OPTIONS_RT;

typedef struct {
	unsigned int		uin;
	char				nick[30];
	char				first[30];
	char				last[30];
	char				email[50];
	int					status;
	unsigned int		flags;
} CONTACT;

//run type data for contacts
typedef struct 
{
	unsigned long	IP;
	unsigned long	REALIP;
}RTOCONTACT;

typedef struct 
{
	unsigned long		xpos;
	unsigned long		ypos;
	unsigned long		width;
	unsigned long		height;
}WINPOS;

typedef struct {
	// poistion on the main windows
	WINPOS				pos_mainwnd;
	WINPOS				pos_sendmsg;
	WINPOS				pos_sendurl;
	WINPOS				pos_recvmsg;
	WINPOS				pos_recvurl;
	// server info
	unsigned long		uin;
	char				password[16];
	char				hostname[64];
	int					port;
	// the contact list
	char				contactlist[MAX_PATH];
	char				history[MAX_PATH];
	CONTACT				clist[MAX_CONTACT];
	RTOCONTACT			clistrto[MAX_CONTACT]; //runtime info about client
	int					ccount;
	// some funky assed flags
	unsigned int		flags1;
	int					alpha;
	// other shit
	int					playsounds;
	int					defaultmode;
	int					autoalpha;

	//hotkeys
	int					hotkey_showhide;
	int					hotkey_setfocus;
	int					hotkey_netsearch;

	// proxy
	int					proxyuse;
	char				proxyhost[128];
	int					proxyport;
	int					proxyauth;
	char				proxyname[128];
	char				proxypass[128];

} OPTIONS;

typedef struct {
	unsigned long uin;
	char *nick;
	char *first;
	char *last;
	char *email;
} INFOREPLY;

typedef struct {
	unsigned long uin;
	char *city;
	unsigned short country_code;
	char country_stat;
	char *state;
	unsigned short age;
	char gender;
	char *phone;
	char *hp;
	char *about;
} EXTINFOREPLY;

typedef struct {
	int count;
	int max;
	char **bodys;
} HISTORY;


//bad ass functions
void LoadOptions(OPTIONS *opts, char *postkey);
void SaveOptions(OPTIONS *opts, char *postkey);

#endif