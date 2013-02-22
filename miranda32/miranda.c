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



// kick ass thingies
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

// include files
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <richedit.h>
#include <time.h>

#include "resource.h"
#include "global.h"

#include "Plugins.h"
#include "Pluginapi.h"

#include "../icqlib/icq.h"

#include "msgque.h"
#include "import.h"


// global variables
HINSTANCE				ghInstance;
HINSTANCE				ghIcons; //hinst of DLL containing Icons
HANDLE					ghInstMutex; //mutex for this instance/profile
HWND					ghwnd;
HWND					hwndModeless;
OPTIONS					opts;
OPTIONS_RT				rto;
//char					*glpCmdLine;
ICQLINK					link, *plink;
char					mydir[MAX_PATH]; //the dir we are operating from (look for files in etc).
										//MUST have a trailing slash (\)
char					myprofile[MAX_PROFILESIZE]; //contains the name of the profile (default is 'default')

//int						connectionAlive; //GOT SOME PROBS, been disabled

//from plugins.c
extern int plugincnt;

//from msgque.c
extern MESSAGE *msgque;
extern int msgquecnt;

// function prototypes
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcMsgRcv(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcMsgSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcUrlSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcPassword(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcSound(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcAdded(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcUrlRecv(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcAddContact1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcAddContact2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcAddContact3(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcResult(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcDetails(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcGenOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcProxy(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//BOOL CALLBACK DlgProcRecvFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void IPtoString(unsigned long IP,char* strIP);

void ChangeStatus(HWND hWnd, int newStatus);
void LoadContactList(OPTIONS *opts, OPTIONS_RT *rto);
void SaveContactList(OPTIONS *opts, OPTIONS_RT *rto);
void ChangeContactStatus(unsigned long uin, unsigned long status);
int GetStatusIconIndex(unsigned long status);
//void DisplayMessageRecv(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, char *msg);
void DisplayMessageRecv(int msgid);
//void DisplayUrlRecv(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, char *msg, char *url);
void SetCaptionToNext(unsigned long uin);
void ShowNextMsg(int mqid); //used with URLs too
void SetGotMsgIcon(unsigned long uin,BOOL gotmsg); //For when a u recved a msg/url
void DisplayUrlRecv(int msgid);
int GetModelessType(HWND hwnd);
void DeleteWindowFromList(HWND hwnd);
char *LookupContactNick(unsigned long uin);
void UpdateUserStatus(void);
void ContactDoubleClicked(HWND hwnd);
void SendIcqMessage(unsigned long uin, char *name);
void AddContactbyIcq(HWND hwnd);
void AddContactbyEmail(HWND hwnd);
void AddContactbyName(HWND hwnd);
int FirstTime(char *lpCmdLine);
void TrayIcon(HWND hwnd, int action);
unsigned long GetDlgUIN(HWND hwnd);
void AddToContactList(unsigned int uin);
unsigned int GetSelectedUIN(HWND hwnd);
void SendIcqUrl(unsigned long uin, char *name);
int GetOpenURLs(char *buffer, int sz);
void RemoveContact(HWND hwnd);
void RenameContact(HWND hwnd);
void OpenResultWindow(void);
void DisplayDetails(HWND hwnd);
void DisplayUINDetails(unsigned int uin);
int InContactList(unsigned int uin);
unsigned int *GetAckPtr(HWND hwnd);
void SetUserFlags(unsigned int uin, unsigned int flg);
unsigned int GetUserFlags(unsigned int uin);
void ClrUserFlags(unsigned int uin, unsigned int flg);
void AddToHistory(unsigned int uin, time_t evt_t, int dir, char *msg);
void DisplayHistory(HWND hwnd, unsigned int uin);
int LoadSound(char *key, char *sound);
int SaveSound(char *key, char *sound);
int BrowseForWave(HWND wnnd);
void ShowHide(); //show/hide ttoggle of main wnd
void SendFiles(HWND hwnd, unsigned long uin);
HWND CheckForDupWindow(unsigned long uin,long wintype); //is there a send msg window already open for that user
void InitMenuIcons();
int GetMenuIconNo(long ID, BOOL *isCheck);
void GetStatusString(int status,BOOL shortmsg,char*buffer);
void tcp_engage(void);
void tcp_disengage(void);
void parseCmdLine(char *p);

// function prototypes for callback functions
void CbLogged(struct icq_link *link);
void CbDisconnected(struct icq_link *link);
void CbRecvMsg(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *msg);
void CbUserOnline(struct icq_link *link, unsigned long uin, unsigned long status, unsigned long ip, unsigned short port, unsigned long real_ip, unsigned char tcp_flag);
void CbUserOffline(struct icq_link *link, unsigned long uin);
void CbUserStatusUpdate(struct icq_link *link, unsigned long uin, unsigned long status);
void CbInfoReply(struct icq_link *link, unsigned long uin, const char *nick, const char *first, const char *last, const char *email, char auth);
void CbSrvAck(struct icq_link *link, unsigned short seq);
void CbRecvUrl(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *url, const char *desc);
void CbRecvAdded(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *nick, const char *first, const char *last, const char *email);
void CbRecvAuthReq(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *nick, const char *first, const char *last, const char *email, const char *reason);
void CbUserFound(struct icq_link *link, unsigned long uin, const char *nick, const char *first, const char *last, const char *email, char auth);
void CbSearchDone(struct icq_link *link);
void CbExtInfoReply(struct icq_link *link, unsigned long uin, const char *city, unsigned short country_code, char country_stat, const char *state, unsigned short age, char gender, const char *phone, const char *hp, const char *about);
void CbWrongPassword(struct icq_link *link);
void CbInvalidUIN(struct icq_link *link);
void CbRequestNotify(struct icq_link *link, unsigned long id, int result, unsigned int length, void *data);
void CbLog(struct icq_link *link, time_t time, unsigned char level, const char *str);

void CbRecvFileReq(struct icq_link *link, unsigned long uin,
       unsigned char hour, unsigned char minute, unsigned char day,
       unsigned char month, unsigned short year, const char *descr,
       const char *filename, unsigned long filesize, unsigned long seq);



int CALLBACK lvSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort); 

typedef BOOL (WINAPI *TmySetLayeredWindowAttributes)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
TmySetLayeredWindowAttributes mySetLayeredWindowAttributes;

// entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    static TCHAR	szAppName[] = MIRANDANAME ;
    HWND			hwnd;
    MSG				msg;	
    WNDCLASS		wndclass;
	HMODULE			hUserDll;
	char			MutexName[50];
	HANDLE			hAccel;
	
	//put init code after the Parsecmdline and multiinstance check (so that it doesn thave to be cleaned up if ths instance already exists)
	
	
	parseCmdLine(_strdup(lpCmdLine));

	if(strlen(myprofile)>MAX_PROFILESIZE)
	{
		MessageBox(NULL,"Your profile name is too long.",MIRANDANAME,MB_OK);
		return FALSE;
	}

	// check for clashing instance
	sprintf(MutexName,"%s-%s",szAppName,myprofile);
	ghInstMutex=CreateMutex(NULL,TRUE,MutexName);
	if (GetLastError()==ERROR_ALREADY_EXISTS)
	{
		HWND hwnd;
		hwnd = FindWindow(MutexName, NULL);
		if (hwnd)
		{
			SendMessage(hwnd, TI_SHOW, 0, 0);
		}		
		return FALSE;
	}

	//GEN INIT & ICQLIB INIT
	memset(&link, 0, sizeof(link));
	plink = &link;
	ghInstance = hInstance;
	
	tcp_engage();
	InitCommonControls();
	

	//OPTIONS STUFF
	strcpy(rto.inifile, mydir);
	strcat(rto.inifile, myprofile);
	strcat(rto.inifile, ".ini");
	
	if (_access(rto.inifile, 00) == 0)
		rto.useini = TRUE;
	else
		rto.useini = FALSE;

	
	LoadOptions(&opts, myprofile);
	if (opts.uin == 0)
	{
		if (FirstTime(myprofile))
		{
			return 0;
		}
	}
	
	memmove(opts.contactlist + strlen(mydir), opts.contactlist, strlen(opts.contactlist) + 1);
	memcpy(opts.contactlist, mydir, strlen(mydir));

	memmove(opts.history + strlen(mydir), opts.history, strlen(opts.history) + 1);
	memcpy(opts.history, mydir, strlen(mydir));	



	hUserDll = LoadLibrary("user32.dll");
	if (hUserDll) mySetLayeredWindowAttributes = (TmySetLayeredWindowAttributes)GetProcAddress(hUserDll, "SetLayeredWindowAttributes");

	ghIcons = LoadLibrary("Icons.dll");
	if (!ghIcons) ghIcons = ghInstance;

	icq_Init(plink, opts.uin, opts.password, MIRANDANAME, FALSE);

	// register callbacks
	link.icq_Logged = CbLogged;
	link.icq_Disconnected = CbDisconnected;
	link.icq_RecvMessage = CbRecvMsg;
	link.icq_UserOnline = CbUserOnline;
	link.icq_UserOffline = CbUserOffline;
	link.icq_UserStatusUpdate = CbUserStatusUpdate;
	link.icq_InfoReply = CbInfoReply;
	link.icq_ExtInfoReply = CbExtInfoReply;
	link.icq_SrvAck = CbSrvAck;
	link.icq_RecvURL = CbRecvUrl;
	link.icq_RecvFileReq=CbRecvFileReq;
	link.icq_RecvAdded = CbRecvAdded;
	link.icq_UserFound = CbUserFound;
	link.icq_SearchDone = CbSearchDone;
	link.icq_RequestNotify = CbRequestNotify;
	link.icq_Log = CbLog;
	link.icq_WrongPassword = CbWrongPassword;
	link.icq_InvalidUIN = CbInvalidUIN;
	link.icq_RecvAuthReq=CbRecvAuthReq;
    
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_MIRANDA));
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = MutexName;

	RegisterClass(&wndclass);

    hwnd = CreateWindowEx(opts.flags1 & FG_TOOLWND ? WS_EX_TOOLWINDOW : 0, 
						  MutexName,   
                          MIRANDANAME,
						  WS_OVERLAPPEDWINDOW,
                          opts.pos_mainwnd.xpos,
                          opts.pos_mainwnd.ypos,
                          opts.pos_mainwnd.width,
                          opts.pos_mainwnd.height,
                          NULL,               
                          LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU)),
                          hInstance,          
                          NULL);           
	ShowWindow(hwnd, nCmdShow);
    
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCEL));
    while (GetMessage (&msg, NULL, 0, 0))
    {
		if (!TranslateAccelerator(hwnd, hAccel, &msg))
		{
			if (!hwndModeless || !IsDialogMessage(hwndModeless, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
    }
	
	//FINISH UP
	SaveContactList(&opts, &rto);
	SaveOptions(&opts, myprofile);
	
	icq_Done(plink);
	tcp_disengage();	

	FreeLibrary(ghIcons);
	FreeLibrary(hUserDll);

    return msg.wParam ;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{     
	RECT rect;
	int pid=0; //used in WM_command for plugins (show options)
	int id;

	MENUITEMINFO lpmii;//POPUP MNU


    switch (msg)
    {
		case TI_CALLBACK:
			if (lParam==WM_MBUTTONUP)
			{
				ShowHide();
			}
			else if (((opts.flags1 & FG_ONECLK) && lParam==WM_LBUTTONUP)
			        || (!(opts.flags1 & FG_ONECLK) && lParam==WM_LBUTTONDBLCLK))
			{
				if (msgquecnt>0)
				{
					//read unread msgs
					//simulate ctrl+shift+i hotkey
					SendMessage(hwnd,WM_HOTKEY,HOTKEY_SETFOCUS,0);
				}
				else
				{
					ShowHide();
				}
			}
			else if (lParam == WM_RBUTTONDOWN)
			{
				HMENU hMenu;
				POINT p;
				hMenu = LoadMenu(ghInstance, MAKEINTRESOURCE(IDR_CONTEXT));
				hMenu = GetSubMenu(hMenu, 0);
				
				GetCursorPos(&p);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
			}
			return TRUE;
		case TI_SORT:
			ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
			return TRUE;
		case TI_SHOW:
			rto.hidden = TRUE;
			ShowHide();
			return TRUE;
		case WM_CREATE:
		{
			RECT			rect;
			LVCOLUMN		col;
			HICON			hicon;		

			ghwnd = hwnd;
			
			//set Timers
			SetTimer(hwnd, TM_KEEPALIVE, 120000, NULL);			
			SetTimer(hwnd, TM_CHECKDATA, 100, NULL);
			SetTimer(hwnd, TM_ONCE, 100, NULL);
			SetTimer(hwnd, TM_AUTOALPHA,250,NULL);


			//install hotkeys
			if (opts.hotkey_showhide!=-1)
				RegisterHotKey(hwnd,HOTKEY_SHOWHIDE,MOD_CONTROL|MOD_SHIFT,opts.hotkey_showhide);

			if (opts.hotkey_setfocus!=-1)
				RegisterHotKey(hwnd,HOTKEY_SETFOCUS,MOD_CONTROL|MOD_SHIFT,opts.hotkey_setfocus);

			if (opts.hotkey_netsearch!=-1)
				RegisterHotKey(hwnd,HOTKEY_NETSEARCH,MOD_CONTROL|MOD_SHIFT,opts.hotkey_netsearch);

			//create the status wnd
			rto.hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "Offline", hwnd, 0);

			GetClientRect(hwnd, &rect);
			rto.himlIcon = ImageList_Create(16, 16, ILC_MASK|ILC_COLOR16, 28, 0);
			//0-7 - Status icons
			//8-9 Unread Icon
			//10-(13) Menu Icons
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_OFFLINE));//IDI_OFFLINE));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_ONLINE));//IDI_ONLINE));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_INVISIBLE));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_FREE4CHAT));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_NA));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_AWAY));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_DND));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_OCCUPIED));
			ImageList_AddIcon(rto.himlIcon, hicon);

			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_UNREAD));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_UNREAD2));
			ImageList_AddIcon(rto.himlIcon, hicon);

			//MNU ICONS -currntly ONLY uses exe, not icons.dll
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPTIONS));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_USERDETAILS));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_FINDUSER));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_HELP));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_CHECKFORUPGRADE));
			ImageList_AddIcon(rto.himlIcon, hicon);
			
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPTIONS_PROXY));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPTIONS_SOUND));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPTIONS_PLUGINS));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPTIONS_GENERAL));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPTIONS_ICQNPASSWORD));
			ImageList_AddIcon(rto.himlIcon, hicon);
			
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MIRANDAWEBSITE));
			ImageList_AddIcon(rto.himlIcon, hicon);

			//popup mnu icons
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_SENDMSG));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_SENDURL));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_HISTORY));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_RENAME));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_DELETE));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_IGNORE));
			ImageList_AddIcon(rto.himlIcon, hicon);
			hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_IMPORT));
			ImageList_AddIcon(rto.himlIcon, hicon);
				
			//Init SysTray Icon
			TrayIcon(hwnd, TI_CREATE);
			//Create COntact List (Listview)
			rto.hwndContact = CreateWindowEx(LVS_EX_INFOTIP,
											WC_LISTVIEW, 
											"Contact List",
											WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_EDITLABELS |LVS_NOCOLUMNHEADER ,// | LVS_EX_INFOTIP,
											0,
											0,
											rect.right - rect.left,
											rect.top - rect.bottom - 21,
											hwnd,
											NULL,
											ghInstance,
											NULL);

			ListView_SetBkColor(rto.hwndContact, GetSysColor(COLOR_3DFACE));
			ListView_SetTextBkColor(rto.hwndContact, GetSysColor(COLOR_3DFACE));
			memset(&col, 0, sizeof(col));
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = "Contact List";
			col.cx = rect.right - rect.left;
			ListView_InsertColumn(rto.hwndContact, 0, &col);
			ListView_SetImageList(rto.hwndContact, rto.himlIcon, LVSIL_SMALL);
			ShowWindow(rto.hwndContact, SW_SHOW);
			UpdateWindow(hwnd); 
			LoadContactList(&opts, &rto);
			ListView_SetItemState(rto.hwndContact, 0, LVIS_SELECTED, LVIS_SELECTED);
			SetWindowPos(hwnd, (opts.flags1 & FG_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			if (opts.flags1 & FG_TRANSP)
			{
				SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
				if (mySetLayeredWindowAttributes) mySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)opts.alpha, LWA_ALPHA);
			}


			//MENU ICONS
			InitMenuIcons();

			//LOAD PLUGINS
			Plugin_Load();
			Plugin_LoadPlugins(hwnd,ghInstance); //this func also adds em to the icq>options>plugins mnu

			return FALSE;
		}
		//APM
		case WM_POWERBROADCAST:
			switch ((DWORD)wParam)
			{
				case PBT_APMSUSPEND:
					//computer (laptop) is suspending, disconnect from ICQ server
					ChangeStatus(hwnd, STATUS_OFFLINE);
					Sleep(200);
					break;
			}
			break;
		//notified of a hotkey pressed
		case WM_HOTKEY: 
			switch ((int)wParam)
			{
			case HOTKEY_SHOWHIDE:
				ShowHide();
				break;
			case HOTKEY_NETSEARCH:
				ShellExecute(ghwnd,"open","Http://QuickSrch.cjb.net","","",SW_SHOW);
				break;
			case HOTKEY_SETFOCUS:
				if ((id=MsgQue_FindFirstUnread())>=0)
				{
					if (CheckForDupWindow(msgque[id].uin,WT_RECVMSG) || CheckForDupWindow(msgque[id].uin,WT_RECVURL))
					{//displayt msg window already open for that user
						goto makemainwndactive;
					}
					else
					{
						if (msgque[id].isurl)
						{
							DisplayUrlRecv(id);
						}
						else 
						{
							DisplayMessageRecv(id);
						}				
					}
				}
				else
				{
makemainwndactive:
					SetForegroundWindow(ghwnd);
					SetFocus(rto.hwndContact);
				}
				break;
			}
			break;
		case WM_TIMER:
			if ((int)wParam == TM_KEEPALIVE)
			{
				if (rto.online) 
					icq_KeepAlive(plink);
				//SetTimer(hwnd, TM_KEEPALIVE, 120000, NULL);
			}
			if ((int)wParam==TM_MSGFLASH)
			{
				TrayIcon(ghwnd, TI_UPDATE);	
			}
			if ((int)wParam == TM_CHECKDATA)
			{
			
			/*
				
				if (rto.net_activity) 
				{
					if (opts.proxyuse)
						icq_HandleProxyResponse(plink);
					else
						icq_HandleServerResponse(plink);
				}
				*/
				//if (opts.proxyuse)
				//		icq_HandleProxyResponse(plink);


				if (rto.net_activity) 
				{
					if (opts.proxyuse)
						icq_Proxy(plink);
					else
						icq_Main(plink);
					
					/*if ((connectionAlive--)<=0) //Decrease Ack counter, if no Ack from server
					{	                        //during a specified period: disconnect
						
						//emulate icqlib msg
						strcpy(stmsg,"Server Timedout!");
						Plugin_NotifyPlugins(PM_ICQDEBUGMSG,(WPARAM)strlen(stmsg),(LPARAM)stmsg);

						CbDisconnected(0);
					}*/
				}
				
			}
			if ((int)wParam == TM_ONCE)
			{
				if (!rto.online && !rto.askdisconnect && opts.defaultmode != -1)
				{
					ChangeStatus(hwnd, opts.defaultmode);
				}
				SetTimer(hwnd, TM_OTHER, 10000, NULL);
				KillTimer(hwnd, TM_ONCE);
			}
			if ((int)wParam == TM_OTHER)
			{
				if (!rto.online && !rto.askdisconnect && opts.defaultmode != -1)
				{
					ChangeStatus(hwnd, opts.defaultmode);
				}
			}
			if ((int)wParam ==TM_TIMEOUT)
			{
				if (!rto.online)
				{
					SendMessage(rto.hwndStatus, WM_SETTEXT, 0, (LPARAM)"Failed (Timed out)");
				}
				KillTimer(hwnd,TM_TIMEOUT);
			}
			if ((int)wParam==TM_AUTOALPHA && opts.autoalpha!=-1)
			{
				long x;
				long y;
				static BOOL lastinwnd=FALSE;
				BOOL inwnd=FALSE;
				RECT myrect;
				
				if (GetForegroundWindow()==hwnd)
				{ goto aminwnd;}

				x=LOWORD(GetMessagePos());
				y=HIWORD(GetMessagePos());
				GetWindowRect(hwnd,&myrect);
				if (x>=myrect.left && x<=myrect.right)
				{//in X
					if (y>=myrect.top && y<=myrect.bottom)
					{ //in Y
aminwnd:
						inwnd=TRUE;
					}

				}
				if (inwnd!=lastinwnd)
				{ //change
					if (inwnd)
					{//make it default alpha
						if (mySetLayeredWindowAttributes) mySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)opts.alpha, LWA_ALPHA);
					}
					else
					{ //out - set to the the alpha
						if (mySetLayeredWindowAttributes) mySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)opts.autoalpha, LWA_ALPHA);
					}
					lastinwnd=inwnd;
				}


			}
			return TRUE;
		case WM_DESTROY:

			TrayIcon(hwnd, TI_DESTROY);
			
			if (rto.online) 
			{
				icq_Logout(plink);
				icq_Disconnect(plink);			
			}
			
			ReleaseMutex(ghInstMutex);

			//remove timers
			KillTimer(hwnd, TM_CHECKDATA);
			KillTimer(hwnd, TM_OTHER);
			KillTimer(hwnd, TM_KEEPALIVE);
			
			//Kill Hotkeys
			UnregisterHotKey(hwnd,HOTKEY_SHOWHIDE);
			UnregisterHotKey(hwnd,HOTKEY_SETFOCUS);
			UnregisterHotKey(hwnd,HOTKEY_NETSEARCH);

			Plugin_ClosePlugins();

			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
			GetClientRect(hwnd, &rect);
			SetWindowPos(rto.hwndContact, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top - 21, SWP_NOZORDER);
			SetWindowPos(rto.hwndStatus, NULL, 0, rect.bottom - 20, rect.right - rect.left, 20, SWP_NOZORDER);

			if (opts.ccount > ListView_GetCountPerPage(rto.hwndContact))
				ListView_SetColumnWidth(rto.hwndContact, 0, rect.right - rect.left - GetSystemMetrics(SM_CXHSCROLL));
			else
				ListView_SetColumnWidth(rto.hwndContact, 0, rect.right - rect.left);

			if (wParam == SIZE_MINIMIZED && (opts.flags1 & FG_MIN2TRAY))
			{
				ShowWindow(hwnd, SW_HIDE);
				rto.hidden = TRUE;
			}
			// drop through
		case WM_MOVE:
			{
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				GetWindowPlacement(hwnd, &wp);
				opts.pos_mainwnd.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
				opts.pos_mainwnd.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
				opts.pos_mainwnd.xpos = wp.rcNormalPosition.left;
				opts.pos_mainwnd.ypos = wp.rcNormalPosition.top;
			}
			return FALSE;
		case WM_CLOSE:
			if (opts.flags1 & FG_TOOLWND)
				ShowHide();
			else
				PostMessage(hwnd, WM_COMMAND,  ID_ICQ_EX,0);
			
			return FALSE;
		//MENU COMMAND
		case WM_COMMAND:
			{
				int uin;
				char *nick;

				switch (LOWORD(wParam))
				{
					case ID_ICQ_EX:
						PostMessage(hwnd, WM_DESTROY, 0, 0);					  
						break;
					case ID_ICQ_STATUS_OFFLINE:
						ChangeStatus(hwnd, STATUS_OFFLINE);
						break;
					case ID_ICQ_STATUS_ONLINE:
						ChangeStatus(hwnd, STATUS_ONLINE);
						break;
					case ID_ICQ_STATUS_AWAY:
						ChangeStatus(hwnd, STATUS_AWAY);
						break;
					case ID_ICQ_STATUS_DND:  
						ChangeStatus(hwnd, STATUS_DND);
						break;
					case ID_ICQ_STATUS_NA:
						ChangeStatus(hwnd, STATUS_NA);
						break;
					case ID_ICQ_STATUS_OCCUPIED:
						ChangeStatus(hwnd, STATUS_OCCUPIED);
						break;
					case ID_ICQ_STATUS_FREECHAT:
						ChangeStatus(hwnd, STATUS_FREE_CHAT);
						break;
					case ID_ICQ_STATUS_INVISIBLE:
						ChangeStatus(hwnd, STATUS_INVISIBLE);
						break;
					case ID_ICQ_ADDCONTACT_BYICQ:
						AddContactbyIcq(hwnd);
						SaveContactList(&opts, &rto);
						break;
					case ID_ICQ_ADDCONTACT_BYNAME:
						AddContactbyName(hwnd);
						SaveContactList(&opts, &rto);
						break;
					case ID_ICQ_ADDCONTACT_BYEMAIL:
						AddContactbyEmail(hwnd);
						SaveContactList(&opts, &rto);
						break;
					case ID_ICQ_ADDCONTACT_IMPORT:
						ImportPrompt();
						break;
					case ID_ICQ_OPTIONS_ICQPASSWORD:
						DialogBox(ghInstance, MAKEINTRESOURCE(IDD_PASSWORD), hwnd, DlgProcPassword);
						break;
					case ID_ICQ_OPTIONS_GENERALOPTIONS:
						ShowWindow(CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_GENERALOPT), ghwnd, DlgProcGenOpts), SW_SHOW);
						break;
					case ID_ICQ_OPTIONS_SOUNDOPTIONS:
						ShowWindow(CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_SOUND), ghwnd, DlgProcSound), SW_SHOW);
						break;				
					//case ID_ICQ_OPTIONS_PROXYSETTINGS:
					//	ShowWindow(CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_PROXY), ghwnd, DlgProcProxy), SW_SHOW);
					//	break;
					case ID_ICQ_VIEWDETAILS:
						if (rto.online)
						{
							DisplayUINDetails(opts.uin);
						}
						else
						{
							MessageBox(hwnd, "You must be online to retieve your details", MIRANDANAME, MB_OK);
						}
						break;
					case ID_HELP_WINDEX:
						ShellExecute(hwnd, "open", HELPSTR, NULL, NULL, SW_SHOWNORMAL);
						break;
					case ID_HELP_CKECKFORUPGRADES:
						ShellExecute(hwnd, "open", VERSIONSTR, NULL, NULL, SW_SHOWNORMAL);
						break;
					case ID_HELP_ABOUT:
						DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, DlgProcAbout);
						break;
					case ID_HELP_MIRANDAWEBSITE:
						ShellExecute(hwnd, "open", MIRANDAWEBSITE, NULL, NULL, SW_SHOWNORMAL);
						break;
					case ID_CTRLENTER:
						if (hwndModeless) SendMessage(hwndModeless, WM_COMMAND, IDOK, 0);						
						break;
					case POPUP_SENDMSG: 
						uin = GetSelectedUIN(rto.hwndContact);
						if (uin)
						{					
							nick = LookupContactNick(uin);
							SendIcqMessage(uin, nick);
						}
						break;
					case POPUP_SENDURL: 
						uin = GetSelectedUIN(rto.hwndContact);
						{					
							nick = LookupContactNick(uin);
							SendIcqUrl(uin, nick);
						}
						break;
					case POPUP_HISTORY: 
						uin = GetSelectedUIN(rto.hwndContact);
						if (uin) DisplayHistory(hwnd, uin);
						break;
					case POPUP_USERDETAILS: 
						DisplayDetails(rto.hwndContact);
						break;
					case POPUP_IGNORE: 
						uin = GetSelectedUIN(rto.hwndContact);
						if (uin) (GetUserFlags(uin) & UF_IGNORE) ? ClrUserFlags(uin, UF_IGNORE) : SetUserFlags(uin, UF_IGNORE);
						break;
					case POPUP_RENAME: 
						RenameContact(rto.hwndContact);
						SaveContactList(&opts, &rto);
						break;
					case POPUP_DELETE: 
						RemoveContact(rto.hwndContact);
						SaveContactList(&opts, &rto);
						break;	
					case ID_TRAY_HIDE: 
						ShowHide();
						break;
					case ID_TRAY_ABOUT: 
						DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, DlgProcAbout);
						break;
					case ID_TRAY_EXIT: 
						PostMessage(hwnd, WM_DESTROY, 0, 0);
						break;
						
					default:
						pid=wParam-IDC_PLUGMNUBASE;
						if (pid>=0 && pid <=plugincnt)
						{
							Plugin_NotifyPlugin(pid,PM_SHOWOPTIONS,0,0);
						}
						break;
				}
			}
			return FALSE;
		//MSG FROM CHILD CONTROL
		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->hwndFrom == rto.hwndContact)
			{
				switch (((LPNMHDR)lParam)->code)
				{
					case LVN_COLUMNCLICK:
					{
						ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
						return FALSE;
					}
					case LVN_GETINFOTIP:
					{
						return FALSE;
					}
					case LVN_ENDLABELEDIT:
					{
						LV_DISPINFO *pdi;
						unsigned int uin;
						int i;
						pdi = (LV_DISPINFO *)lParam;
						uin = pdi->item.lParam;

						for (i = 0; i < opts.ccount; i++)
						{
							if (uin == opts.clist[i].uin && pdi->item.pszText)
							{								
								strcpy(opts.clist[i].nick, pdi->item.pszText);								
							}
						}
						PostMessage(hwnd, TI_SORT, 0, 0);
						return TRUE;
					}
					case LVN_KEYDOWN:
					{
						LPNMLVKEYDOWN pnkd;
						pnkd= (LPNMLVKEYDOWN) lParam;
						if (pnkd->wVKey==13)
						{//enter keypressed
							ContactDoubleClicked(hwnd);
						}
						else if (pnkd->wVKey==46) //del key
						{
							RemoveContact(rto.hwndContact);
						}
						else if (pnkd->wVKey==VK_F2)
						{
							RenameContact(rto.hwndContact);
						}
						return FALSE;
					}
					case NM_DBLCLK:
						ContactDoubleClicked(hwnd);
						break;
					
				}
			}
			return FALSE;
		case WM_CONTEXTMENU:
			
			if (ListView_GetSelectedCount(rto.hwndContact))
			{
				HMENU hMenu;
				hMenu = LoadMenu(ghInstance, MAKEINTRESOURCE(IDR_CONTEXT));
				hMenu = GetSubMenu(hMenu, 1);

				lpmii.cbSize     = sizeof(lpmii);
				if (!rto.online)
				{				
					lpmii.fMask		 = MIIM_STATE;
					lpmii.fState     = MFS_GRAYED;

					SetMenuItemInfo(hMenu,POPUP_SENDMSG,FALSE,&lpmii);
					SetMenuItemInfo(hMenu,POPUP_SENDURL,FALSE,&lpmii);
					SetMenuItemInfo(hMenu,POPUP_USERDETAILS,FALSE,&lpmii);
				}	
				if (GetUserFlags(GetSelectedUIN(rto.hwndContact)) & UF_IGNORE)
				{
					lpmii.fMask		= MIIM_STATE;
					lpmii.fState	= MFS_CHECKED;

					SetMenuItemInfo(hMenu,POPUP_IGNORE,FALSE,&lpmii);
				}
				else
				{
					lpmii.fMask		= MIIM_STATE;
					lpmii.fState	= MFS_CHECKED;

					SetMenuItemInfo(hMenu,POPUP_IGNORE,FALSE,&lpmii);
				}
			
				lpmii.fMask      = MIIM_BITMAP|MIIM_DATA;
				lpmii.hbmpItem   = HBMMENU_CALLBACK;
				lpmii.dwItemData = MFT_OWNERDRAW;

				SetMenuItemInfo(hMenu,POPUP_SENDMSG,FALSE,&lpmii);
				SetMenuItemInfo(hMenu,POPUP_SENDURL,FALSE,&lpmii);
				SetMenuItemInfo(hMenu,POPUP_HISTORY,FALSE,&lpmii);
				SetMenuItemInfo(hMenu,POPUP_USERDETAILS,FALSE,&lpmii);
				SetMenuItemInfo(hMenu,POPUP_IGNORE,FALSE,&lpmii);
				SetMenuItemInfo(hMenu,POPUP_RENAME,FALSE,&lpmii);
				SetMenuItemInfo(hMenu,POPUP_DELETE,FALSE,&lpmii);

				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, LOWORD(lParam), HIWORD(lParam), 0, hwnd, NULL);
								
			}
			return TRUE;

		//DRAW ICONS IN MENU
		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT hItem;
			int i; //Number of image in rto.himlIcons
			int isCheck = FALSE;
			hItem=(DRAWITEMSTRUCT*)lParam;
			i=GetMenuIconNo(hItem->itemID,&isCheck);
			//if (hItem->itemData==DRAW_ICONONLY)
			
			if (hItem->itemData==MFT_OWNERDRAW)
			{
				if (isCheck==TRUE)
				{
					if(hItem->itemState & (ODS_SELECTED))
					{
						ImageList_DrawEx(rto.himlIcon,i,hItem->hDC,0,0,0,0,CLR_NONE,CLR_DEFAULT,ILD_SELECTED);
					}
					else if (hItem->itemState & (ODS_CHECKED))
					{
						ImageList_DrawEx(rto.himlIcon,i,hItem->hDC,0,0,0,0,CLR_NONE,CLR_NONE,ILD_NORMAL);
					}			
					else
					{
						ImageList_DrawEx(rto.himlIcon,i,hItem->hDC,0,0,0,0,CLR_NONE,GetSysColor(COLOR_MENU),ILD_SELECTED);
					}
				}
				else 
				{
					ImageList_DrawEx(rto.himlIcon,i,hItem->hDC,0,0,0,0,CLR_NONE,CLR_NONE,ILD_NORMAL);
				}
			}
			else
			{
				//DrawMenuItem(hItem, i, isCheck);
			}	
			return TRUE;
			break;	
		}
		//WIDTH OF ICONS IN MENU
		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT hItem;
			hItem=(MEASUREITEMSTRUCT*)lParam;
			if (hItem->itemData==MFT_OWNERDRAW)
			{
				hItem->itemWidth=0;    // width is without checkmark, which we overwrite
				hItem->itemHeight=16;  // miniicons
				return TRUE;
			}
		}

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

BOOL CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));
			return TRUE;			
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					EndDialog(hwndDlg, 0);
					return TRUE;
			}
	}
	return 0;
}

BOOL CALLBACK DlgProcMsgRecv(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem;
	DLG_DATA *dlgdata;
	
	char buffer[128];
	char *buf;
	
	WINDOWPLACEMENT wp;//wmsize/wmmove

	unsigned long uin;

	switch (msg)
	{
		case WM_INITDIALOG:

			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_RECVMSG)));
			
			dlgdata = (DLG_DATA *)lParam;
			buf = LookupContactNick(dlgdata->uin);
			
			if (buf)
				sprintf(buffer,"Message from %s",buf);
			else
				sprintf(buffer,"Message from %d",dlgdata->uin);

			SetWindowText(hwndDlg,buffer);
			

			
			//SetProp(hwndDlg,"MQID",dlgdata->msgid);

			hwndItem = GetDlgItem(hwndDlg, IDC_MSG);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)dlgdata->msg);
			hwndItem = GetDlgItem(hwndDlg, IDC_FROM);
			
			if (buf)
			{
				sprintf(buffer, "%s", buf);
			}
			else
			{
				sprintf(buffer, "%d", dlgdata->uin);
			}
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_DATE);
			sprintf(buffer, "%d:%2.2d %d/%2.2d/%4.4d", dlgdata->hour, dlgdata->minute, dlgdata->day, dlgdata->month, dlgdata->year);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
			sprintf(buffer, "%d", dlgdata->uin);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			
			if (!InContactList(dlgdata->uin)) EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), TRUE);
			
			if (MsgQue_MsgsForUIN(dlgdata->uin)>1 )
			{//more messages in the que, make Close to NExt
				SetDlgItemText(hwndDlg, IDCANCEL, "&Next");
			}

			if (opts.pos_recvmsg.xpos!=-1)
				MoveWindow(hwndDlg,opts.pos_recvmsg.xpos,opts.pos_recvmsg.ypos,opts.pos_recvmsg.width,opts.pos_recvmsg.height,TRUE);

			return TRUE;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_SIZE:
		case WM_MOVE:
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			opts.pos_recvmsg.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			opts.pos_recvmsg.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			opts.pos_recvmsg.xpos = wp.rcNormalPosition.left;
			opts.pos_recvmsg.ypos = wp.rcNormalPosition.top;
			break;
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDCANCEL:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);

					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);

					ShowNextMsg(MsgQue_FindFirst(uin));
					return TRUE;
				case IDOK:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					buf = LookupContactNick(uin);
					SendIcqMessage(uin, buf);

					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					
					ShowNextMsg(MsgQue_FindFirst(uin));
					return TRUE;
				case IDC_ADD:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					AddToContactList(uin);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
					return TRUE;
				case IDC_DETAILS:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					DisplayUINDetails(uin);
					return TRUE;
				case IDC_HISTORY:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 1024, (LPARAM)buffer);
					uin = atol(buffer);
					DisplayHistory(hwndDlg, uin);
					return TRUE;
			}
	}
	return FALSE;
}

BOOL CALLBACK DlgProcUrlRecv(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem;
	DLG_DATA *dlgdata;
	static int mqid;

	WINDOWPLACEMENT wp;//wmsize/wmmove

	char buffer[1024];
	char *buf;

	unsigned long uin;

	switch (msg)
	{
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_RECVURL)));

			dlgdata = (DLG_DATA *)lParam;

			buf = LookupContactNick(dlgdata->uin);
			
			if (buf)
				sprintf(buffer,"URL from %s",buf);
			else
				sprintf(buffer,"URL from %d",dlgdata->uin);

			SetWindowText(hwndDlg,buffer);

			mqid=dlgdata->msgid;
			hwndItem = GetDlgItem(hwndDlg, IDC_MSG);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)dlgdata->msg);
			hwndItem = GetDlgItem(hwndDlg, IDC_URL);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)dlgdata->url);
			hwndItem = GetDlgItem(hwndDlg, IDC_FROM);
			

			if (buf)
			{
				sprintf(buffer, "%s", buf);
			}
			else
			{
				sprintf(buffer, "%d", dlgdata->uin);
			}
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_DATE);
			sprintf(buffer, "%d:%2.2d %d/%2.2d/%4.4d", dlgdata->hour, dlgdata->minute, dlgdata->day, dlgdata->month, dlgdata->year);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
			sprintf(buffer, "%d", dlgdata->uin);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);

			if (MsgQue_MsgsForUIN(dlgdata->uin)>1 )
			{//more messages in the que, make Close to NExt
				SetDlgItemText(hwndDlg, IDCANCEL, "&Next");
			}

			if (opts.pos_recvurl.xpos!=-1)
				MoveWindow(hwndDlg,opts.pos_recvurl.xpos,opts.pos_recvurl.ypos,opts.pos_recvurl.width,opts.pos_recvurl.height,TRUE);

			return TRUE;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_SIZE:
		case WM_MOVE:
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			opts.pos_recvurl.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			opts.pos_recvurl.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			opts.pos_recvurl.xpos = wp.rcNormalPosition.left;
			opts.pos_recvurl.ypos = wp.rcNormalPosition.top;
			break;
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					ShowNextMsg(mqid);
					return TRUE;
				case IDOK:
					GetDlgItemText(hwndDlg, IDC_URL, buffer, sizeof(buffer));
					ShellExecute(hwndDlg, "open", buffer, NULL, NULL, SW_SHOW);
					return TRUE;
				case IDC_REPLY:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					buf = LookupContactNick(uin);
					SendIcqMessage(uin, buf);

					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					ShowNextMsg(mqid);
					return TRUE;
			}
	}
	return FALSE;
}

BOOL CALLBACK DlgProcMsgSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem;
	DLG_DATA *dlgdata;
	
	WINDOWPLACEMENT wp;//wmsize/wmmove

	unsigned long uin;
	char buffer[1024];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, IDC_BODY, EM_LIMITTEXT, 450, 0);
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_SENDMSG)));
			dlgdata = (DLG_DATA *)lParam;
			hwndItem = GetDlgItem(hwndDlg, IDC_NAME);			

			if (dlgdata->nick)
				sprintf(buffer,"Send Message to %s",dlgdata->nick);
			else
				sprintf(buffer,"Send Message to %d",dlgdata->uin);

			SetWindowText(hwndDlg,buffer);


			if (dlgdata->nick)
			{
				sprintf(buffer, "%s [%d]", dlgdata->nick,dlgdata->uin);
			}
			else
			{
				sprintf(buffer, "%d", dlgdata->uin);
			}
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
			sprintf(buffer, "%d", dlgdata->uin);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);

			if (opts.pos_sendmsg.xpos!=-1)
				MoveWindow(hwndDlg,opts.pos_sendmsg.xpos,opts.pos_sendmsg.ypos,opts.pos_sendmsg.width,opts.pos_sendmsg.height,TRUE);

			return TRUE;
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
				{
					unsigned long *srvack;

					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 1024, (LPARAM)buffer);
					uin = atol(buffer);
					
					hwndItem = GetDlgItem(hwndDlg, IDC_BODY);
					SendMessage(hwndItem, WM_GETTEXT, 1024, (LPARAM)buffer);

					srvack = GetAckPtr(hwndDlg);
					*srvack = icq_SendMessage(plink, uin, buffer, ICQ_SEND_THRUSERVER);
					ShowWindow(hwndDlg, SW_SHOWMINIMIZED);
					AddToHistory(uin, time(NULL), HISTORY_MSGSEND, buffer);
					return TRUE;
				}
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDC_HISTORY:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 1024, (LPARAM)buffer);
					uin = atol(buffer);
					DisplayHistory(hwndDlg, uin);
					return TRUE;
			}
			break;
		case WM_SIZE:
		case WM_MOVE:
			
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			opts.pos_sendmsg.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			opts.pos_sendmsg.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			opts.pos_sendmsg.xpos = wp.rcNormalPosition.left;
			opts.pos_sendmsg.ypos = wp.rcNormalPosition.top;
			break;
		case TI_SRVACK:
			DeleteWindowFromList(hwndDlg);
			DestroyWindow(hwndDlg);
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcUrlSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem;
	DLG_DATA *dlgdata;

	WINDOWPLACEMENT wp;//wmsize/wmmove

	unsigned long uin;
	char msgbuf[1028];
	char urlbuf[128];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			char buffer[2048];
			char *token;
			GetOpenURLs(buffer, sizeof(buffer));
			token = strtok(buffer, "\n");
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghIcons, MAKEINTRESOURCE(IDI_SENDURL)));
			while (token)
			{
				SendDlgItemMessage(hwndDlg, IDC_URLS, CB_ADDSTRING, 0, (LPARAM)token);
				token = strtok(NULL, "\n");
			}
			SendDlgItemMessage(hwndDlg, IDC_URLS, CB_SETCURSEL, 0, 0);

			dlgdata = (DLG_DATA *)lParam;
			
			if (dlgdata->nick)
				sprintf(buffer,"Send URL to %s",dlgdata->nick);
			else
				sprintf(buffer,"Send URL to %d",dlgdata->uin);

			SetWindowText(hwndDlg,buffer);


			hwndItem = GetDlgItem(hwndDlg, IDC_NAME);			
			if (dlgdata->nick)
			{
				sprintf(buffer, "%s [%d]", dlgdata->nick,dlgdata->uin);
			}
			else
			{
				sprintf(buffer, "%d", dlgdata->uin);
			}
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
			sprintf(buffer, "%d", dlgdata->uin);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);

			if (opts.pos_sendurl.xpos!=-1)
				MoveWindow(hwndDlg,opts.pos_sendurl.xpos,opts.pos_sendurl.ypos,opts.pos_sendurl.width,opts.pos_sendurl.height,TRUE);

			return TRUE;
		}
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
				{
					unsigned long *srvack;
					char buffer[2048];

					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)urlbuf);
					uin = atol(urlbuf);
					
					hwndItem = GetDlgItem(hwndDlg, IDC_BODY);
					SendMessage(hwndItem, WM_GETTEXT, 1024, (LPARAM)msgbuf);

					hwndItem = GetDlgItem(hwndDlg, IDC_URLS);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)urlbuf);

					srvack = GetAckPtr(hwndDlg);
					*srvack = icq_SendURL(plink, uin, urlbuf, msgbuf, ICQ_SEND_THRUSERVER);

					sprintf(buffer, "%s\r\n%s", urlbuf, msgbuf);
					AddToHistory(uin, time(NULL), HISTORY_URLSEND, buffer);
					return TRUE;
				}
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
			}
			break;
		case WM_SIZE:
		case WM_MOVE:
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			opts.pos_sendurl.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			opts.pos_sendurl.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			opts.pos_sendurl.xpos = wp.rcNormalPosition.left;
			opts.pos_sendurl.ypos = wp.rcNormalPosition.top;
			break;
		case TI_SRVACK:
			DeleteWindowFromList(hwndDlg);
			DestroyWindow(hwndDlg);
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcAddContact1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem; 

	unsigned int uin;
	char buffer[128];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));
			return TRUE;
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);

					AddToContactList(uin);

					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
			}
	}
	return FALSE;
}

BOOL CALLBACK DlgProcAddContact2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem; 

	char buffer[128];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));
			return TRUE;
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					hwndItem = GetDlgItem(hwndDlg, IDC_EMAIL);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);

					icq_SendSearchReq(plink, buffer, "", "", "");
					
					OpenResultWindow();

					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
			}
	}
	return FALSE;
}

BOOL CALLBACK DlgProcAddContact3(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char nick[128];
	char first[128];
	char last[128];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));
			return TRUE;
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					GetDlgItemText(hwndDlg, IDC_NICK, nick, 128);
					GetDlgItemText(hwndDlg, IDC_FIRST, first, 128);
					GetDlgItemText(hwndDlg, IDC_LAST, last, 128);

					icq_SendSearchReq(plink, "", nick, first, last);
					
					OpenResultWindow();

					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
			}
	}
	return FALSE;
}

BOOL CALLBACK DlgProcPassword(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndChild;

	unsigned long uin;
	char buffer[32];

	switch (msg)
	{
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));
			return TRUE;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;			
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDCANCEL:
					EndDialog(hwndDlg, 1);
					return TRUE;
				case IDOK:
					hWndChild = GetDlgItem(hwndDlg, IDC_ICQNUM);
					SendMessage(hWndChild, WM_GETTEXT, 32, (LPARAM)buffer);
					uin = atol(buffer);
					opts.uin = uin;
					hWndChild = GetDlgItem(hwndDlg, IDC_PASSWORD);
					SendMessage(hWndChild, WM_GETTEXT, 32, (LPARAM)buffer);
					strcpy(opts.password, buffer);
					SaveOptions(&opts, myprofile);

					
					icq_Disconnect(plink);
					if (plink->icq_Uin!=0) //if its zero, dont bother (jsut get acces violation)
						icq_Done(plink);

					icq_Init(plink, opts.uin, opts.password, MIRANDANAME, TRUE);

					EndDialog(hwndDlg, 0);
					return TRUE;
			}
	}
	return 0;
}

BOOL CALLBACK DlgProcGPL(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FILE *fp;

	char *buf;

	switch (msg)
	{
		case WM_INITDIALOG:			
			{
				char fn[MAX_PATH];
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));

				strcpy(fn, mydir);
				strcat(fn, "gpl.txt");
				fp = fopen("gpl.txt", "rb");
				if (fp)
				{
					buf = calloc(32 * 1024, 1);

					fread(buf, 1, 32 * 1024, fp);

					SendDlgItemMessage(hwndDlg, IDC_TEXT, WM_SETTEXT, 0, (LPARAM)buf);
					free(buf);
					fclose(fp);
				}
			}
			return TRUE;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;			
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				case IDOK:
					EndDialog(hwndDlg, 3);
					return TRUE;
			}
	}
	return 0;
}

BOOL CALLBACK DlgProcSound(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buf[MAX_PATH];

	switch (msg)
	{
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));

			if (opts.playsounds) 
				SendDlgItemMessage(hwndDlg, IDC_USESOUND, BM_SETCHECK, BST_CHECKED, 0);
			
			if (LoadSound("MICQ_Loged", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND1, buf);			
			if (LoadSound("MICQ_Disconnected", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND2, buf);
			if (LoadSound("MICQ_RecvMsg", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND3, buf);
			if (LoadSound("MICQ_RecvUrl", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND4, buf);
			if (LoadSound("MICQ_UserOnline", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND5, buf);
			if (LoadSound("MICQ_UserOffline", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND6, buf);
			if (LoadSound("MICQ_Added", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND7, buf);
			if (LoadSound("MICQ_AuthReq", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND8, buf);
			if (LoadSound("MICQ_Error", buf))
				SetDlgItemText(hwndDlg, IDC_SOUND9, buf);

			return TRUE;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				case IDOK:
					opts.playsounds = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_USESOUND, BM_GETCHECK, 0, 0)) ? 1 : 0;
					GetDlgItemText(hwndDlg, IDC_SOUND1, buf, MAX_PATH);
					SaveSound("MICQ_Loged", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND2, buf, MAX_PATH);
					SaveSound("MICQ_Disconnected", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND3, buf, MAX_PATH);
					SaveSound("MICQ_RecvMsg", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND4, buf, MAX_PATH);
					SaveSound("MICQ_RecvUrl", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND5, buf, MAX_PATH);
					SaveSound("MICQ_UserOnline", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND6, buf, MAX_PATH);
					SaveSound("MICQ_UserOffline", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND7, buf, MAX_PATH);
					SaveSound("MICQ_Added", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND8, buf, MAX_PATH);
					SaveSound("MICQ_AuthReq", buf);
					GetDlgItemText(hwndDlg, IDC_SOUND9, buf, MAX_PATH);
					SaveSound("MICQ_Error", buf);
					EndDialog(hwndDlg, 3);
					return TRUE;
				case IDC_B1:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND1));
					break;
				case IDC_B2:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND2));
					break;
				case IDC_B3:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND3));
					break;
				case IDC_B4:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND4));
					break;
				case IDC_B5:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND5));
					break;
				case IDC_B6:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND6));
					break;
				case IDC_B7:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND7));
					break;
				case IDC_B8:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND8));
					break;
				case IDC_B9:
					BrowseForWave(GetDlgItem(hwndDlg, IDC_SOUND9));
					break;
			}
	}
	return 0;
}

BOOL CALLBACK DlgProcAdded(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem;
	DLG_DATA *dlgdata;

	unsigned long uin;
	char buffer[128];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE)));
			dlgdata = (DLG_DATA *)lParam;
			hwndItem = GetDlgItem(hwndDlg, IDC_NAME);			
			if (dlgdata->nick)
			{
				sprintf(buffer, "%s", dlgdata->nick);
			}
			else
			{
				sprintf(buffer, "%d", dlgdata->uin);
			}
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
			sprintf(buffer, "%d", dlgdata->uin);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			if (InContactList(dlgdata->uin)) EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
			return TRUE;
		}
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					AddToContactList(uin);
					
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);

					return TRUE;
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcAuthReq(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem;
	DLG_DATA *dlgdata;

	unsigned long uin;
	char buffer[128];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MIRANDA)));
			dlgdata = (DLG_DATA *)lParam;
			if (dlgdata->nick)
			{
				sprintf(buffer, "%s", dlgdata->nick);
			}
			else
			{
				sprintf(buffer, "%d", dlgdata->uin);
			}
			hwndItem = GetDlgItem(hwndDlg, IDC_NAME);			
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
			sprintf(buffer, "%d", dlgdata->uin);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);
			hwndItem = GetDlgItem(hwndDlg, IDC_REASON);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)dlgdata->msg);
			return TRUE;
		}
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					icq_SendAuthMsg(plink, uin);
					
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);

					return TRUE;
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcResult(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			HWND hwnd;
			LVCOLUMN		col;

			hwnd = GetDlgItem(hwndDlg, IDC_CONTACTS);

			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = "UIN";
			col.cx = 100;
			ListView_InsertColumn(hwnd, 0, &col);

			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = "Nickname";
			col.cx = 100;
			ListView_InsertColumn(hwnd, 1, &col);

			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = "First Name";
			col.cx = 100;
			ListView_InsertColumn(hwnd, 2, &col);

			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = "Last Name";
			col.cx = 100;
			ListView_InsertColumn(hwnd, 3, &col);
			return TRUE;
		}
		case TI_USERFOUND:
		{
			char buffer[128];
			HWND hwnd;
			int count;

			LV_ITEM item;
			DLG_DATA *dlg_data = (DLG_DATA *)lParam;

			hwnd = GetDlgItem(hwndDlg, IDC_CONTACTS);				
			count = ListView_GetItemCount(hwnd);

			ZeroMemory(&item, sizeof(item));
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iSubItem = 0;
			item.lParam = dlg_data->uin;
			sprintf(buffer, "%d", dlg_data->uin);
			item.pszText = buffer;
			count = ListView_InsertItem(hwnd, &item);			

			if (dlg_data->nick)
			{
				ListView_SetItemText(hwnd, count, 1, (char *)dlg_data->nick);
			}
			if (dlg_data->first)
			{
				ListView_SetItemText(hwnd, count, 2, (char *)dlg_data->first);
			}
			if (dlg_data->last)
			{
				ListView_SetItemText(hwnd, count, 3, (char *)dlg_data->last);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
				{
					int idx;
					HWND hwnd;
					LVITEM item;
					hwnd = GetDlgItem(hwndDlg, IDC_CONTACTS);

					idx = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
					if (idx != -1)
					{
						item.mask = LVIF_PARAM;
						item.iItem = idx;

						ListView_GetItem(hwnd, &item);
						AddToContactList(item.lParam);
					}					
					return TRUE;
				}
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcDetails(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buffer[128];

	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			int i=0;
			char sip[16];
			char srealip[16];
			char smsg[50];
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			
			//IP
			for (i = 0; i < opts.ccount; i++)
			{
				if (lParam == (long)opts.clist[i].uin)
				{
					if (opts.clistrto[i].IP == 0 || opts.clistrto[i].REALIP == 0)
					{
						sprintf(smsg,"<NA>");
					}
					else
					{
						IPtoString(opts.clistrto[i].IP,sip);
						IPtoString(opts.clistrto[i].REALIP,srealip);
						if (_stricmp(srealip,sip)!=0)
							sprintf(smsg,"%s [Real IP:%s]",sip,srealip);
						else
							sprintf(smsg,"%s",sip);

					}
					SetDlgItemText(hwndDlg,IDC_IP,smsg);
					break;
				}
			}

			return TRUE;
		}
		case TI_INFOREPLY:
		{
			INFOREPLY *inforeply = (INFOREPLY *)lParam;
			
			if (inforeply->uin == (unsigned int)GetWindowLong(hwndDlg, GWL_USERDATA))
			{
				sprintf(buffer, "User Details (%d)", inforeply->uin);
				SetWindowText(hwndDlg, buffer);
				SetDlgItemText(hwndDlg, IDC_FIRST, inforeply->first);
				SetDlgItemText(hwndDlg, IDC_LAST, inforeply->last);
				SetDlgItemText(hwndDlg, IDC_NICK, inforeply->nick);
				SetDlgItemText(hwndDlg, IDC_EMAIL, inforeply->email);
			}
			break;
		}
		case TI_EXTINFOREPLY:
		{
			EXTINFOREPLY *extinforeply = (EXTINFOREPLY *)lParam;
			if (extinforeply->uin == (unsigned int)GetWindowLong(hwndDlg, GWL_USERDATA))
			{
				SetDlgItemText(hwndDlg, IDC_HP, extinforeply->hp);
				SetDlgItemText(hwndDlg, IDC_ABOUT, extinforeply->about);
				SetDlgItemText(hwndDlg, IDC_CITY, extinforeply->city);
				SetDlgItemText(hwndDlg, IDC_PHONE, extinforeply->phone);
				SetDlgItemText(hwndDlg, IDC_STATE, extinforeply->state);
				SetDlgItemText(hwndDlg, IDC_COUNTRY, icq_GetCountryName(extinforeply->country_code));

				if (extinforeply->gender == 2) SetDlgItemText(hwndDlg, IDC_GENDER, "M");
				if (extinforeply->gender == 1) SetDlgItemText(hwndDlg, IDC_GENDER, "F");

				sprintf(buffer, "%d", extinforeply->age);
				SetDlgItemText(hwndDlg, IDC_AGE, buffer);
			}
			break;
		}
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DeleteWindowFromList(hwndDlg);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDC_GO:
					GetDlgItemText(hwndDlg, IDC_HP, buffer, 128);
					if (*buffer)
					{
						if (strncmp(buffer, "http", 4) && strncmp(buffer, "HTTP", 4))
						{
							memmove(buffer + 7, buffer, strlen(buffer));
							strncpy(buffer, "http://", 7);
						}
						ShellExecute(hwndDlg, "open", buffer, NULL, NULL, SW_SHOW);
					}
					return TRUE;
				case IDC_GO2:
					strcpy(buffer, "mailto:");
					GetDlgItemText(hwndDlg, IDC_EMAIL, buffer + strlen(buffer), 128 - strlen(buffer));
					if (*(buffer+7)) ShellExecute(hwndDlg, "open", buffer, NULL, NULL, SW_SHOW);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcProxy(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwndDlg, IDC_USE,  BM_SETCHECK, opts.proxyuse  ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_AUTH, BM_SETCHECK, opts.proxyauth ? BST_CHECKED : BST_UNCHECKED, 0);

			SetDlgItemText(hwndDlg, IDC_HOST, opts.proxyhost);
			SetDlgItemInt(hwndDlg, IDC_PORT, opts.proxyport, FALSE);
			SetDlgItemText(hwndDlg, IDC_NAME, opts.proxyname);
			SetDlgItemText(hwndDlg, IDC_PASS, opts.proxypass);

			return TRUE;
		}
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					opts.proxyauth = SendDlgItemMessage(hwndDlg, IDC_USE, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0;

					GetDlgItemText(hwndDlg, IDC_HOST, opts.proxyhost, sizeof(opts.hostname));
					GetDlgItemText(hwndDlg, IDC_NAME, opts.proxyname, sizeof(opts.proxyname));
					GetDlgItemText(hwndDlg, IDC_PASS, opts.proxypass, sizeof(opts.proxypass));
					opts.proxyport = GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE);

					if ((SendDlgItemMessage(hwndDlg, IDC_USE, BM_GETCHECK, 0, 0) == BST_CHECKED) && !opts.proxyuse)
					{
						icq_SetProxy(plink, opts.proxyhost, (unsigned short)opts.proxyport, opts.proxyauth, opts.proxyname, opts.proxypass);
					}
					if (!(SendDlgItemMessage(hwndDlg, IDC_USE, BM_GETCHECK, 0, 0) == BST_CHECKED) && opts.proxyuse)
					{
						icq_UnsetProxy(plink);
					}
					opts.proxyuse  = SendDlgItemMessage(hwndDlg, IDC_USE, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0;


					SaveOptions(&opts, myprofile);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;
			}
			break;
	}
	return FALSE;
}



BOOL CALLBACK DlgProcGenOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwndDlg, IDC_MIN2TRAY, BM_SETCHECK, (opts.flags1 & FG_MIN2TRAY) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_ONTOP, BM_SETCHECK, (opts.flags1 & FG_ONTOP) ? BST_CHECKED : BST_UNCHECKED, 0);			
			SendDlgItemMessage(hwndDlg, IDC_TRANSP, BM_SETCHECK, (opts.flags1 & FG_TRANSP) ? BST_CHECKED : BST_UNCHECKED, 0);

			SendDlgItemMessage(hwndDlg, IDC_POPUP, BM_SETCHECK, (opts.flags1 & FG_POPUP) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_TOOLWND, BM_SETCHECK, (opts.flags1 & FG_TOOLWND) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_ONECLK, BM_SETCHECK, (opts.flags1 & FG_ONECLK) ? BST_CHECKED : BST_UNCHECKED, 0);
			
			SendDlgItemMessage(hwndDlg, IDC_USEINI, BM_SETCHECK, rto.useini ? BST_CHECKED : BST_UNCHECKED, 0);
			return TRUE;
		}
		case WM_COMMAND:
			switch ((int)wParam)
			{
				case IDOK:
					opts.flags1 = 0;
					opts.flags1 |= SendDlgItemMessage(hwndDlg, IDC_MIN2TRAY, BM_GETCHECK, 0, 0) ? FG_MIN2TRAY : 0;
					opts.flags1 |= SendDlgItemMessage(hwndDlg, IDC_ONTOP, BM_GETCHECK, 0, 0) ? FG_ONTOP : 0;					
					opts.flags1 |= SendDlgItemMessage(hwndDlg, IDC_TRANSP, BM_GETCHECK, 0, 0) ? FG_TRANSP : 0;
					opts.flags1 |= SendDlgItemMessage(hwndDlg, IDC_POPUP, BM_GETCHECK, 0, 0) ? FG_POPUP : 0;
					opts.flags1 |= SendDlgItemMessage(hwndDlg, IDC_TOOLWND, BM_GETCHECK, 0, 0) ? FG_TOOLWND : 0;
					opts.flags1 |= SendDlgItemMessage(hwndDlg, IDC_ONECLK, BM_GETCHECK, 0, 0) ? FG_ONECLK : 0;
					
					rto.useini = SendDlgItemMessage(hwndDlg, IDC_USEINI, BM_GETCHECK, 0, 0) ? TRUE : FALSE;
					if (!rto.useini) remove(rto.inifile);

					SetWindowPos(ghwnd, (opts.flags1 & FG_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
					if (opts.flags1 & FG_TRANSP)
					{
						SetWindowLong(ghwnd, GWL_EXSTYLE, GetWindowLong(ghwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						if (mySetLayeredWindowAttributes) mySetLayeredWindowAttributes(ghwnd, RGB(0,0,0), (BYTE)opts.alpha, LWA_ALPHA);
						RedrawWindow(ghwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
					}
					else
					{
						SetWindowLong(ghwnd, GWL_EXSTYLE, GetWindowLong(ghwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
						RedrawWindow(ghwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
					}
					if (opts.flags1 & FG_TOOLWND)
					{
						ShowWindow(ghwnd, SW_HIDE);
						SetWindowLong(ghwnd, GWL_EXSTYLE, GetWindowLong(ghwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
						ShowWindow(ghwnd, SW_NORMAL);
					}
					else
					{
						ShowWindow(ghwnd, SW_HIDE);
						SetWindowLong(ghwnd, GWL_EXSTYLE, GetWindowLong(ghwnd, GWL_EXSTYLE) & ~WS_EX_TOOLWINDOW);
						ShowWindow(ghwnd, SW_NORMAL);
					}

					SaveOptions(&opts, myprofile);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			return TRUE;
		case WM_INITDIALOG:
		{
			int i = 0;
			int sz;
			char buf[4096];
			char uinb[32];
			FILE *fp;
			HISTORY *hs;

			sprintf(uinb, "%d", lParam);
			hs = malloc(sizeof(HISTORY));
			hs->bodys = malloc(sizeof(char *) * 100);
			hs->max = 100;
			hs->count = 0;
			SetWindowLong(hwndDlg, GWL_USERDATA, (long)hs);
			if ((fp = fopen(opts.history, "rb")) == NULL) return TRUE;
			while (fgets(buf, sizeof(buf), fp))
			{
				if (!strncmp(buf, uinb, strlen(uinb)))
				{
					buf[strlen(buf)-4] = 0;
					SendDlgItemMessage(hwndDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)(strchr(buf, '#')+1));
					*buf = 0;
					sz = strlen(buf);
					while (fgets(buf + sz, sizeof(buf) - sz, fp) && strncmp(buf + sz, "оооо", 4))
					{
						sz = strlen(buf);
					}
					buf[strlen(buf)-6] = 0;
					if (i == hs->max)
					{
						hs->max *= 2;
						hs->bodys = realloc(hs->bodys, sizeof(char *) * hs->max);
					}
					hs->bodys[i++] = _strdup(buf);
				}
				else
				{
					while(fgets(buf, sizeof(buf), fp) && strncmp(buf, "оооо", 4));
				}
			}
			hs->count = i;
			if (i)
			{
				SendDlgItemMessage(hwndDlg, IDC_LIST, LB_SETCURSEL, i - 1, 0);
				SetDlgItemText(hwndDlg, IDC_EDIT, hs->bodys[i-1]);
			}
			fclose(fp);
			return TRUE;
		}
		case WM_DESTROY:
		{
			HISTORY *hs;
			int i;
			hs = (HISTORY *)GetWindowLong(hwndDlg, GWL_USERDATA);
			for (i = 0; i < hs->count; i++)
			{
				free(hs->bodys[i]);
			}
			free(hs->bodys);
			free(hs);
			return FALSE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDC_LIST:
					if (HIWORD(wParam) == LBN_SELCHANGE)
					{
						int i;
						HISTORY *hs;
						hs = (HISTORY *)GetWindowLong(hwndDlg, GWL_USERDATA);
						i = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
						SetDlgItemText(hwndDlg, IDC_EDIT, hs->bodys[i]);
					}
					return TRUE;
			}
			break;
	}
	return FALSE;
}


/*BOOL CALLBACK DlgProcRecvFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndItem;
	DLG_DATA *dlgdata;
	char buffer[128];
	char *buf;
	unsigned long uin;

	switch (msg)
	{
		case WM_INITDIALOG:
			dlgdata = (DLG_DATA *)lParam;
			buf = LookupContactNick(dlgdata->uin);
			
			hwndItem = GetDlgItem(hwndDlg, IDC_FROM);
			if (buf)
			{
				sprintf(buffer, "%s", buf);
			}
			else
			{
				sprintf(buffer, "%d", dlgdata->uin);
			}
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);

			hwndItem = GetDlgItem(hwndDlg, IDC_DATE);
			sprintf(buffer, "%d:%2.2d %d/%2.2d/%4.4d", dlgdata->hour, dlgdata->minute, dlgdata->day, dlgdata->month, dlgdata->year);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);

			if (!InContactList(dlgdata->uin)) EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), TRUE);

			hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
			sprintf(buffer, "%d", dlgdata->uin);
			SendMessage(hwndItem, WM_SETTEXT, 0, (LPARAM)buffer);

			break;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				hwndModeless = hwndDlg;
			}
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				hwndModeless = NULL;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDC_ADD:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					AddToContactList(uin);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
					return TRUE;
				case IDC_DETAILS:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 128, (LPARAM)buffer);
					uin = atol(buffer);
					DisplayUINDetails(uin);
					return TRUE;
				case IDC_HISTORY:
					hwndItem = GetDlgItem(hwndDlg, IDC_ICQ);
					SendMessage(hwndItem, WM_GETTEXT, 1024, (LPARAM)buffer);
					uin = atol(buffer);
					DisplayHistory(hwndDlg, uin);
					return TRUE;
			}
	}

}
*/

VOID ChangeStatus(HWND hWnd, int newStatus)
{
	char conmsg[30];
	char tmpst[25];
	rto.status = newStatus;

	if (newStatus == STATUS_OFFLINE)
	{
		rto.askdisconnect = TRUE;
		opts.defaultmode = -1;
		if (rto.online) 
		{

			icq_Logout(plink);
			icq_Disconnect(plink);
		}
	}
	else
	{
		if (!rto.online)
		{
			//connectionAlive=60;
			//set a timer for timeout
			
			GetStatusString(newStatus,TRUE,tmpst);
			sprintf(conmsg,"Connecting...[%s]",tmpst);
			SendMessage(rto.hwndStatus, WM_SETTEXT, 0, (LPARAM)conmsg);
			SetTimer(ghwnd,TM_TIMEOUT,TIMEOUT_TIMEOUT,NULL);
			rto.net_activity = TRUE;
			if (icq_Connect(plink, opts.hostname, opts.port)==-1)
			{
				SendMessage(rto.hwndStatus, WM_SETTEXT, 0, (LPARAM)"Error Connecting");
				KillTimer(ghwnd,TM_TIMEOUT);
			}
			icq_Login(plink, newStatus);
		}
		else
		{
			UpdateUserStatus();
			icq_ChangeStatus(plink, newStatus);
		}
		opts.defaultmode = newStatus;
	}
}

void LoadContactList(OPTIONS *opts, OPTIONS_RT *rto)
{
	LVITEM item;

	FILE *fp;

	if (opts->contactlist) fp = fopen(opts->contactlist, "rb");

	if (fp)
	{
		while (fread(&opts->clist[opts->ccount], sizeof(CONTACT), 1, fp))
		{
			opts->clist[opts->ccount].status = STATUS_OFFLINE & 0xFFFF;

			if (rto->hwndContact)
			{
				memset(&item, 0, sizeof(item));
				if (opts->clist[opts->ccount].nick[0] == 0)
				{
					item.pszText = malloc(32);
					sprintf(item.pszText, "%d", opts->clist[opts->ccount].uin);
				}
				else
				{
					item.pszText = _strdup(opts->clist[opts->ccount].nick);
				}
				item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
				item.iItem = opts->ccount;
				item.iImage = 0;
				item.iSubItem = 0;
				item.lParam = opts->clist[opts->ccount].uin;
				ListView_InsertItem(rto->hwndContact, &item);				
			}
			icq_ContactAdd(plink, opts->clist[opts->ccount].uin);
			opts->ccount++;
		}
		fclose(fp);
	}
	ListView_SortItems(rto->hwndContact, lvSortFunc, ++rto->sortmode);
}

void SaveContactList(OPTIONS *opts, OPTIONS_RT *rto)
{
	FILE *fp;

	int i;

	if (opts->contactlist) fp = fopen(opts->contactlist, "wb");

	if (fp)
	{
		for (i = 0; i < opts->ccount; i++)
		{			
			fwrite(&opts->clist[i], sizeof(CONTACT), 1, fp);
		}
		fclose(fp);
	}
}

void ChangeContactStatus(unsigned long uin, unsigned long status)
{
	LVITEM item;
	LVFINDINFO findinfo;
	unsigned long oldstatus;
	int i;
	int idx;

	for (i = 0; i < opts.ccount; i++)
	{
		if (uin == opts.clist[i].uin)
		{
			oldstatus=opts.clist[i].status;
			opts.clist[i].status = status & 0xFFFF;

			if (MsgQue_MsgsForUIN(uin)>0)
			{//the Unread Msg icon will be there, so dont change their icon
				
			}
			else
			{
				findinfo.flags = LVFI_PARAM;
				findinfo.lParam = uin;
				idx = ListView_FindItem(rto.hwndContact, -1, &findinfo);
				
				if (idx != -1)
				{
					item.mask = LVIF_IMAGE;
					item.iItem = idx;
					item.iImage = GetStatusIconIndex(status);
					item.iSubItem = 0;			
					ListView_SetItem(rto.hwndContact, &item);
				}
			}
			//TELL PLugins someone has changed status
			Plugin_NotifyPlugins(PM_CONTACTSTATUSCHANGE, (WPARAM)oldstatus,(LPARAM)&opts.clist[i]);//lparam pointer to contact struct
		}
	}

	
}

int GetStatusIconIndex(unsigned long status)
{
	int index;

	switch (status & 0xFFFF)
	{
		case STATUS_OFFLINE: 
		case 0xFFFF:
			index = 0;
			break;
		case STATUS_ONLINE:
			index = 1;
			break;
		case STATUS_INVISIBLE:
			index = 2;
			break;
		case STATUS_NA:
		case 0x0005:
			index = 4;
			break;
		case STATUS_FREE_CHAT:
			index = 3;
			break;
		case STATUS_OCCUPIED:
		case 0x0011:
			index = 7; //4;
			break;
		case STATUS_AWAY:
			index = 5;
			break;
		case STATUS_DND:
		case 0x0013:
			index = 6; //4;
			break;
		default:
			index = 1;
			break;
	}
	return index;
}

//void DisplayMessageRecv(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, char *msg)
void DisplayMessageRecv(int msgid)
{
	//msg stays in array untill the close btn has pressed(not when the window is opend)
	HWNDLIST *node;
	DLG_DATA dlg_data;

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_RECVMSG;
	node->next = rto.hwndlist;
	node->uin = msgque[msgid].uin;
	rto.hwndlist = node;

	dlg_data.uin = msgque[msgid].uin;
	dlg_data.hour = msgque[msgid].hour;
	dlg_data.minute = msgque[msgid].minute;
	dlg_data.day = msgque[msgid].day;
	dlg_data.month = msgque[msgid].month;
	dlg_data.year = msgque[msgid].year;
	dlg_data.msg = msgque[msgid].msg;
	
	dlg_data.msgid=msgid;
	msgque[msgid].beingread=TRUE;

	node->hwnd = CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_MSGRECV), NULL, DlgProcMsgRecv, (LPARAM)&dlg_data);
	//ShowWindow(node->hwnd, (opts.flags1 & FG_POPUP) ? SW_SHOW : SW_SHOWMINIMIZED);
	ShowWindow(node->hwnd,  SW_SHOW);
	SetForegroundWindow(node->hwnd);
}

void DisplayUrlRecv(int msgid)
{
	HWNDLIST *node;
	DLG_DATA dlg_data;

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_RECVURL;
	node->next = rto.hwndlist;
	node->uin = msgque[msgid].uin;
	rto.hwndlist = node;

	dlg_data.uin = msgque[msgid].uin;
	dlg_data.hour = msgque[msgid].hour;
	dlg_data.minute = msgque[msgid].minute;
	dlg_data.day = msgque[msgid].day;
	dlg_data.month = msgque[msgid].month;
	dlg_data.year = msgque[msgid].year;
	
	dlg_data.descr = msgque[msgid].msg;
	dlg_data.msg = msgque[msgid].msg;

	dlg_data.url = msgque[msgid].url;
	dlg_data.msgid=msgid;
	
	msgque[msgid].beingread=TRUE;
	
	node->hwnd = CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_URLRECV), NULL, DlgProcUrlRecv, (LPARAM)&dlg_data);
	//ShowWindow(node->hwnd, (opts.flags1 & FG_POPUP) ? SW_SHOW : SW_SHOWMINIMIZED);
	ShowWindow(node->hwnd,  SW_SHOW);
	SetForegroundWindow(node->hwnd);
}

int GetModelessType(HWND hwnd)
{
	HWNDLIST *node;

	int type;

	node = rto.hwndlist;
	while (node)
	{
		if (hwnd == node->hwnd)
		{
			type = node->windowtype;
			hwnd = NULL;
		}
		else
		{
			node = node->next;
		}
	}
	return type;
}

unsigned int *GetAckPtr(HWND hwnd)
{
	HWNDLIST *node = rto.hwndlist;
	while (node)
	{
		if (hwnd == node->hwnd)
		{
			return &(node->ack);
		}
		node = node->next;
	}
	return NULL;
}

void DeleteWindowFromList(HWND hwnd)
{
	HWNDLIST *node;
	HWNDLIST *curr;

	node = rto.hwndlist;
	if (node->hwnd == hwnd)
	{
		curr = node;

		rto.hwndlist = curr->next;

		free(curr);
		return;
	}
	else
	{
		while (node)
		{
			if (hwnd == node->next->hwnd)
			{
				curr = node->next;
				node->next = curr->next;

				free(curr);
				return;
			}
			node = node->next;
		}
	}
}

char *LookupContactNick(unsigned long uin)
{
	int i;

	for (i = 0; i < opts.ccount; i++)
	{
		if (uin == opts.clist[i].uin)
		{
			if (*opts.clist[i].nick == 0)
			{
				return NULL;
			}
			else
			{
				return opts.clist[i].nick;
			}
		}
	}
	return NULL;
}

void UpdateUserStatus(void)
{
	HMENU hmenu;

	char buffer[128];
	unsigned long status;

	hmenu = GetMenu(ghwnd);
	hmenu = GetSubMenu(hmenu, 1);

	status = rto.status;

	CheckMenuItem(hmenu, ID_ICQ_STATUS_OFFLINE, MF_UNCHECKED);
	CheckMenuItem(hmenu, ID_ICQ_STATUS_ONLINE, MF_UNCHECKED);
	CheckMenuItem(hmenu, ID_ICQ_STATUS_INVISIBLE, MF_UNCHECKED);
	CheckMenuItem(hmenu, ID_ICQ_STATUS_NA, MF_UNCHECKED);
	CheckMenuItem(hmenu, ID_ICQ_STATUS_FREECHAT, MF_UNCHECKED);
	CheckMenuItem(hmenu, ID_ICQ_STATUS_OCCUPIED, MF_UNCHECKED);
	CheckMenuItem(hmenu, ID_ICQ_STATUS_AWAY, MF_UNCHECKED);
	CheckMenuItem(hmenu, ID_ICQ_STATUS_DND, MF_UNCHECKED);

	/*switch (status)
	{
		case STATUS_OFFLINE:
			sprintf(buffer, "%s", "Offline");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_OFFLINE, MF_CHECKED);
			break;
		case STATUS_ONLINE:
			sprintf(buffer, "%s", "Online");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_ONLINE, MF_CHECKED);
			break;
		case STATUS_INVISIBLE:
//		case STATUS_INVISIBLE_2:
			sprintf(buffer, "%s", "Online: Invisible");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_INVISIBLE, MF_CHECKED);
			break;
		case STATUS_NA:
			sprintf(buffer, "%s", "Online: NA");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_NA, MF_CHECKED);
			break;
		case STATUS_FREE_CHAT:
			sprintf(buffer, "%s", "Online: Free Chat");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_FREECHAT, MF_CHECKED);
			break;
		case STATUS_OCCUPIED:
			sprintf(buffer, "%s", "Online: Occupied");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_OCCUPIED, MF_CHECKED);
			break;
		case STATUS_AWAY:
			sprintf(buffer, "%s", "Online: Away");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_AWAY, MF_CHECKED);
			break;
		case STATUS_DND:
			sprintf(buffer, "%s", "Online: DND");
			CheckMenuItem(hmenu, ID_ICQ_STATUS_DND, MF_CHECKED);
			break;
	}	*/
	GetStatusString(status,FALSE,buffer);
	
	SendMessage(rto.hwndStatus, WM_SETTEXT, 0, (LPARAM)buffer);

	TrayIcon(ghwnd, TI_UPDATE);

	//TELL PLugins we have changed oour status
	Plugin_NotifyPlugins(PM_STATUSCHANGE,(WPARAM)status,0);//wparam, new status
}

void GetStatusString(int status,BOOL shortmsg,char*buffer)
{
	
	switch (status)
	{
		case STATUS_OFFLINE:
			sprintf(buffer, "%s", "Offline");
			break;
		case STATUS_ONLINE:
			sprintf(buffer, "%s", "Online");
			break;
		case STATUS_INVISIBLE:
//		case STATUS_INVISIBLE_2:
			if (shortmsg)
				sprintf(buffer, "%s", "Invisible");
			else
				sprintf(buffer, "%s", "Online: Invisible");
			
			break;
		case STATUS_NA:
			if (shortmsg)
				sprintf(buffer, "%s", "NA");
			else
				sprintf(buffer, "%s", "Online: NA");

			break;
		case STATUS_FREE_CHAT:
			if (shortmsg)
				sprintf(buffer, "%s", "Free Chat");
			else
				sprintf(buffer, "%s", "Online: Free Chat");

			break;
		case STATUS_OCCUPIED:
			if (shortmsg)
				sprintf(buffer, "%s", "Occupied");
			else
				sprintf(buffer, "%s", "Online: Occupied");

			break;
		case STATUS_AWAY:
			if (shortmsg)
				sprintf(buffer, "%s", "Away");
			else
				sprintf(buffer, "%s", "Online: Away");

			break;
		case STATUS_DND:
			if (shortmsg)
				sprintf(buffer, "%s", "DND");
			else
				sprintf(buffer, "%s", "Online: DND");
			break;
	}	

	
	
}
void ContactDoubleClicked(HWND hwnd)
{
	LVITEM item;
	int mqid;
	int selcount;
	int idx;
	unsigned long uin;
	char *name;

	if (rto.online)
	{
		selcount = ListView_GetSelectedCount(rto.hwndContact);
		if (selcount == 1)
		{
			idx = ListView_GetNextItem(rto.hwndContact, -1, LVNI_ALL | LVNI_SELECTED);
			if (idx != -1)
			{
				item.mask = LVIF_PARAM;
				item.iItem = idx;

				ListView_GetItem(rto.hwndContact, &item);
				uin = item.lParam;
				if (CheckForDupWindow(uin,WT_RECVMSG)!=NULL || CheckForDupWindow(uin,WT_RECVURL)!=NULL || MsgQue_MsgsForUIN(uin)==0)
				{ //no msgs waiting, or its alreadt being displyed, so a send msg instead
					name = LookupContactNick(uin);
					SendIcqMessage(uin, name);
				}
				else
				{//read msg/url
					
					mqid=MsgQue_FindFirst(uin);
					if (msgque[mqid].isurl)
					{
						DisplayUrlRecv(mqid);
					}
					else 
					{
						DisplayMessageRecv(mqid);
					}
				}
				
				
		
			}
		}
	}
}

unsigned int GetSelectedUIN(HWND hwnd)
{
	LVITEM item;
	int idx;
	unsigned int uin = 0;

	idx = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
	if (idx != -1)
	{
		item.mask = LVIF_PARAM;
		item.iItem = idx;

		ListView_GetItem(hwnd, &item);
		uin = item.lParam;
	}
	return uin;
}

void SendIcqMessage(unsigned long uin, char *name)
{
	HWNDLIST *node;
	DLG_DATA dlg_data;

	HWND wnd;
	if ((wnd=CheckForDupWindow(uin,WT_SENDMSG))!=NULL)
	{
		SetForegroundWindow(wnd);
		return;
	}


	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_SENDMSG;
	node->next = rto.hwndlist;
	node->uin = uin;
	rto.hwndlist = node;

	dlg_data.uin = uin;
	dlg_data.nick = name;
	
	node->hwnd = CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_MSGSEND), NULL, DlgProcMsgSend, (LPARAM)&dlg_data);
	ShowWindow(node->hwnd, SW_SHOW);
}

void SendIcqUrl(unsigned long uin, char *name)
{
	HWNDLIST *node;
	DLG_DATA dlg_data;

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_SENDURL;
	node->next = rto.hwndlist;
	node->uin = uin;
	rto.hwndlist = node;

	dlg_data.uin = uin;
	dlg_data.nick = name;
	
	node->hwnd = CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_SENDURL), NULL, DlgProcUrlSend, (LPARAM)&dlg_data);
	ShowWindow(node->hwnd, SW_SHOW);
}

void AddContactbyIcq(HWND hwnd)
{
	HWNDLIST *node;

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_ADDCONT;
	node->next = rto.hwndlist;
	rto.hwndlist = node;

	node->hwnd = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_ADDCONTACT1), ghwnd, DlgProcAddContact1);
	ShowWindow(node->hwnd, SW_SHOW);	
}

void AddContactbyEmail(HWND hwnd)
{
	if (rto.online)
	{
		HWNDLIST *node;

		node = malloc(sizeof(HWNDNODE));

		node->windowtype = WT_ADDCONT;
		node->next = rto.hwndlist;
		rto.hwndlist = node;

		node->hwnd = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_ADDCONTACT2), ghwnd, DlgProcAddContact2);
		ShowWindow(node->hwnd, SW_SHOW);	
	}
	else
	{
		MessageBox(hwnd, "You must be online to search for contacts", MIRANDANAME, MB_OK);
	}
}

void AddContactbyName(HWND hwnd)
{
	if (rto.online)
	{
		HWNDLIST *node;

		node = malloc(sizeof(HWNDNODE));

		node->windowtype = WT_ADDCONT;
		node->next = rto.hwndlist;
		rto.hwndlist = node;

		node->hwnd = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_ADDCONTACT3), ghwnd, DlgProcAddContact3);
		ShowWindow(node->hwnd, SW_SHOW);	
	}
	else
	{
		MessageBox(hwnd, "You must be online to search for contacts", MIRANDANAME, MB_OK);
	}
}

void OpenResultWindow(void)
{
	HWNDLIST *node;

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_RESULT;
	node->next = rto.hwndlist;
	rto.hwndlist = node;

	node->hwnd = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_RESULTS), ghwnd, DlgProcResult);
	ShowWindow(node->hwnd, SW_SHOW);
}

int FirstTime(char *lpCmdLine)
{
	// display the gpl
	if (DialogBox(ghInstance, MAKEINTRESOURCE(IDD_GNUPL), NULL, DlgProcGPL) != 3)
	{
		return 1;
	}

	// get the UIN and password
	if (DialogBox(ghInstance, MAKEINTRESOURCE(IDD_PASSWORD), NULL, DlgProcPassword) == 1)
	{
		return 1;
	}

	sprintf(opts.contactlist, "%d.cnt", opts.uin);

	sprintf(opts.history, "%d.hst", opts.uin);

	return 0;
}

void TrayIcon(HWND hwnd, int action)
{
	NOTIFYICONDATA nid;
	static int fc=8;
	nid.cbSize = sizeof(nid);
	nid.hWnd = hwnd;
	nid.uID = 69;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = TI_CALLBACK;
	nid.hIcon = NULL;
	if (rto.online)
	{
		//nid.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYONLINE));
		nid.hIcon=ImageList_GetIcon(rto.himlIcon,GetStatusIconIndex(rto.status),ILD_NORMAL);
	}
	else
	{
		//nid.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_TRAYOFFLINE));
		nid.hIcon=ImageList_GetIcon(rto.himlIcon,GetStatusIconIndex(STATUS_OFFLINE),ILD_NORMAL);
	}
	if (msgquecnt>0 && MsgQue_ReadCount()!=msgquecnt) //if the readcnt=msgquecnyt then all the msgs are being read
	{//have unread msgs, show msg icon in sys
	
			if (nid.hIcon) DestroyIcon(nid.hIcon);
			nid.hIcon=ImageList_GetIcon(rto.himlIcon,fc,ILD_NORMAL);
			if (fc==8)
				fc=9; //unread(2) flash
			else
				fc=8; //unread

			SetTimer(ghwnd,TM_MSGFLASH,TIMEOUT_MSGFLASH,NULL);
	}
	else
	{
		KillTimer(ghwnd,TM_MSGFLASH);
	}

	if (opts.uin)
	{
		sprintf(nid.szTip, "Miranda ICQ: %d", opts.uin);
	}
	else
	{
		strcpy(nid.szTip, "Miranda ICQ");
	}
	switch (action)
	{
		case TI_CREATE:
			Shell_NotifyIcon(NIM_ADD, &nid);
			break;
		case TI_DESTROY:
			Shell_NotifyIcon(NIM_DELETE, &nid);
			break;
		case TI_UPDATE:
			nid.uFlags = NIF_ICON;
			Shell_NotifyIcon(NIM_MODIFY, &nid);
			break;
	}
	DestroyIcon(nid.hIcon);
}

unsigned long GetDlgUIN(HWND hwnd)
{
	HWNDLIST *node;

	node = rto.hwndlist;
	while (node)
	{
		if (hwnd == node->hwnd)
		{
			return node->uin;
		}
		else
		{
			node = node->next;
		}
	}
	return 0;
}

void AddToContactList(unsigned int uin)
{
	LVITEM item;

	if (uin)
	{
		// add the guy to the contact list
		opts.clist[opts.ccount].uin = uin;
		strcpy(opts.clist[opts.ccount].nick, "");
		strcpy(opts.clist[opts.ccount].first, "");
		strcpy(opts.clist[opts.ccount].last, "");
		strcpy(opts.clist[opts.ccount].email, "");
		opts.clist[opts.ccount].status = STATUS_OFFLINE & 0xFFFF;
		// add the guy to the list view
		if (rto.hwndContact)
		{
			item.pszText = malloc(32);
			sprintf(item.pszText, "%d", opts.clist[opts.ccount].uin);						
			item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
			item.iItem = opts.ccount;
			item.iImage = 0;
			item.iSubItem = 0;
			item.lParam = opts.clist[opts.ccount].uin;
			ListView_InsertItem(rto.hwndContact, &item);				
		}
		// do network stuff
		icq_ContactAdd(plink, opts.clist[opts.ccount].uin);
		if (rto.online)
		{
			icq_SendContactList(plink);
			icq_SendInfoReq(plink, uin);
		}
		opts.ccount++;
	}
	SaveContactList(&opts, &rto);
	ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
}

void RemoveContact(HWND hwnd)
{
	LVITEM item;
	int idx;
	int i;
	unsigned int uin;
	if (MessageBox(ghwnd,"Do you really want to delete this contact?",MIRANDANAME,MB_YESNO)==IDNO){return;}
	idx = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
	if (idx != -1)
	{
		item.mask = LVIF_PARAM;
		item.iItem = idx;
		
		ListView_GetItem(hwnd, &item);
		uin = item.lParam;

		ListView_DeleteItem(hwnd, idx);
		
		for (i = 0; i < opts.ccount; i++)
		{
			if (uin == opts.clist[i].uin)
			{
				opts.clist[i] = opts.clist[--opts.ccount];
				return;
			}
		}
				
	}
	ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
}

void DisplayDetails(HWND hwnd)
{
	LVITEM item;
	int idx;
	unsigned int uin;

	idx = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
	if (idx != -1)
	{
		item.mask = LVIF_PARAM;
		item.iItem = idx;

		ListView_GetItem(hwnd, &item);
		uin = item.lParam;

		DisplayUINDetails(uin);
	}
}

void DisplayUINDetails(unsigned int uin)
{
	HWNDLIST *node;

	icq_SendInfoReq(plink, uin);
	icq_SendExtInfoReq(plink, uin);	
	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_DETAILS;
	node->next = rto.hwndlist;
	rto.hwndlist = node;

	node->hwnd = CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_DETAILS), ghwnd, DlgProcDetails, uin);
	ShowWindow(node->hwnd, SW_SHOW);
}

void RenameContact(HWND hwnd)
{
	int idx;

	idx = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
	if (idx != -1)
	{
		SetFocus(hwnd);
		ListView_EditLabel(hwnd, idx);
	}
}

int InContactList(unsigned int uin)
{
	int i;

	for (i = 0; i < opts.ccount; i++)
	{
		if (opts.clist[i].uin == uin) return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK ChildControlProc(HWND hwnd, LPARAM lParam)
{
	int sz;
	int res;
	char buf[128];
	
	res = SendMessage(hwnd, WM_GETTEXT, 128, (LPARAM)buf);
	buf[res] = 0;
	
	if (res && (!strncmp(buf, "http", 4) || !strncmp(buf, "ftp", 3)))   {
		sz = strlen(buf) + strlen((char *)(lParam + sizeof(int))) + 1;     if (sz <= *((int *)lParam) && !strstr((char *)(lParam + sizeof(int)),buf))
		{
			strcat((char *)(lParam + sizeof(int)), buf);
			strcat((char *)(lParam + sizeof(int)), "\n");
		}
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK BrowserWindowProc(HWND hwnd, LPARAM lParam)
{
	char buf[128];
	
	SendMessage(hwnd, WM_GETTEXT, 128, (LPARAM)buf);
	
	if (strstr(buf, "Internet Explorer") ||
		strstr(buf, "Netscape") ||
		strstr(buf, "Opera") ||
		strstr(buf, "Mozilla"))
	{
		EnumChildWindows(hwnd, ChildControlProc, lParam);
	}
	return TRUE;
}

// parameters are a string to get the data and the length of the string // urls are '\n' delimited.
// returns 0 on error or the length of the string
int GetOpenURLs(char *buffer, int sz)
{
	if (sz > (sizeof(int) + 1))
	{
		*((int *)buffer) = sz;
		buffer[sizeof(int)] = 0;
		
		EnumDesktopWindows(NULL, BrowserWindowProc, (LPARAM)buffer); 
		
		memmove(buffer, buffer + sizeof(int), strlen(buffer + sizeof(int))+1); 
		
		return strlen(buffer);
	}
	else
	{
		*buffer = '\0';
		return 0;
	}
}

unsigned int GetUserFlags(unsigned int uin)
{
	int i;
	for (i = 0; i < opts.ccount; i++)
	{
		if (uin == opts.clist[i].uin) return opts.clist[i].flags;
	}
	return 0;
}

void SetUserFlags(unsigned int uin, unsigned int flg)
{
	int i;
	for (i = 0; i < opts.ccount; i++)
	{
		if (uin == opts.clist[i].uin) opts.clist[i].flags |= flg;
	}
}

void ClrUserFlags(unsigned int uin, unsigned int flg)
{
	int i;
	for (i = 0; i < opts.ccount; i++)
	{
		if (uin == opts.clist[i].uin) opts.clist[i].flags &= ~flg;
	}
}

void AddToHistory(unsigned int uin, time_t evt_t, int dir, char *msg)
{
	FILE *fp;
	struct tm *et;
	char dirstr[30];
	et = localtime(&evt_t);
	fp = fopen(opts.history, "ab");
	
	/*if (dir == '<') dirstr = "Incoming message from";
	if (dir == '>') dirstr = "Outgoing message to";*/

	switch (dir)
	{
	case HISTORY_MSGRECV:
		strcpy(dirstr,"Incoming message from");
		break;
	case HISTORY_MSGSEND:
		strcpy(dirstr,"Outgoing message to");
		break;
	case HISTORY_URLRECV:
		strcpy(dirstr,"Incoming URL from");
		break;
	case HISTORY_URLSEND:
		strcpy(dirstr,"Outgoing URL to");
		break;
	}

	fprintf(fp, "%d#%s %s at %d:%2.2d on %2.2d/%2.2d/%4.4d %c\r\n", uin, dirstr, LookupContactNick(uin), et->tm_hour, et->tm_min, et->tm_mday, et->tm_mon + 1, et->tm_year + 1900, dir);
	fprintf(fp, "%s\r\n", msg);
	fprintf(fp, "%c%c%c%c\r\n", 0xEE, 0xEE, 0xEE, 0xEE);	

	fclose(fp);
}

void DisplayHistory(HWND hwnd, unsigned int uin)
{
	ShowWindow(CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_HISTORY), hwnd, DlgProcHistory, uin), SW_SHOW);
}

time_t MakeTime(int hour, int minute, int day, int month, int year)
{
	struct tm et;
	memset(&et, 0, sizeof(et));
	et.tm_hour = hour - 1;
	et.tm_min = minute;
	et.tm_mday = day;
	et.tm_mon = month - 1;
	et.tm_year = year - 1900;

	return mktime(&et);
}

void PlaySoundEvent(char *event)
{
	char sound[MAX_PATH];
	 
	if (opts.playsounds && LoadSound(event, sound) && *sound)
	{
		PlaySound(sound, NULL, SND_ASYNC | SND_FILENAME);
	}
}

int LoadSound(char *key, char *sound)
{
	HKEY hkey;
	char szKey[MAX_PATH];
	DWORD cbSound;
	int res;
	sound[0]=0; //just in case we dont get anything from the reg

	res = FALSE;	
	cbSound = MAX_PATH;
		
	sprintf(szKey, "AppEvents\\Schemes\\Apps\\MirandaICQ\\%s\\.current", key);
		
	if (RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS) 
	{
		if(RegQueryValueEx(hkey, NULL, 0, NULL, sound, &cbSound) ==	ERROR_SUCCESS)
		{
			res = TRUE;
		}
		RegCloseKey(hkey);
	}
	return res;
}

int SaveSound(char *key, char *sound)
{
	HKEY hkey;
	char szKey[MAX_PATH];
	DWORD cbSound;
	DWORD dsp;
	int res;

	res = FALSE;	
	cbSound = MAX_PATH;
		
	sprintf(szKey, "AppEvents\\Schemes\\Apps\\MirandaICQ\\%s\\.current", key);
		
	if (RegCreateKeyEx(HKEY_CURRENT_USER, szKey, 0, NULL, 0, KEY_WRITE, NULL, &hkey, &dsp) == ERROR_SUCCESS) 
	{
		cbSound = strlen(sound);
		if(RegSetValueEx(hkey, NULL, 0, REG_SZ, sound, cbSound) ==	ERROR_SUCCESS)
		{
			res = TRUE;
		}
		RegCloseKey(hkey);
	}
	return res;
}

int BrowseForWave(HWND hwnd)
{
	char buf[MAX_PATH] = {0};

	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetParent(hwnd);
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Wave Files\0*.WAV\0";
	ofn.lpstrFile = buf;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.nMaxFile = sizeof(buf);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "wav";

	if (GetOpenFileName(&ofn))
		SetWindowText(hwnd, buf);

	return 0;
}

// start: call back functions

void CbLogged(struct icq_link *link)
{
	int i;

	rto.askdisconnect = FALSE;
	rto.online = TRUE;

	PlaySoundEvent("MICQ_Logged");

	UpdateUserStatus();

	// make sure everybody has info
	for (i = 0; i < opts.ccount; i++)
	{
		if (opts.clist[i].nick[0] == 0)
		{
			icq_SendInfoReq(plink, opts.clist[i].uin);
		}
	}
	// send me contact list for good luck
	icq_SendContactList(plink);

	TrayIcon(ghwnd, TI_UPDATE);
}

void CbDisconnected(struct icq_link *link)
{
	int i;

	rto.online = FALSE;
	rto.net_activity = FALSE;
	rto.status = STATUS_OFFLINE;

	PlaySoundEvent("MICQ_Disconnected");

	UpdateUserStatus();

	// hey, put everybody offline
	for (i = 0; i < opts.ccount; i++)
	{
		ChangeContactStatus(opts.clist[i].uin, STATUS_OFFLINE);
	}
	TrayIcon(ghwnd, TI_UPDATE);
}

void CbRecvMsg(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *msg)
{
	int mqid=0;
	

	
	AddToHistory(uin, MakeTime(hour, minute, day, month, year), HISTORY_MSGRECV, (char *)msg);

	if (!(GetUserFlags(uin) & UF_IGNORE))
	{
		PlaySoundEvent("MICQ_RecvMsg");

		mqid=MsgQue_Add(uin,hour,minute,day,month,year,(char *)msg,NULL,FALSE);
		
		SetGotMsgIcon(uin,TRUE);
		
		if (MsgQue_MsgsForUIN(uin)==1)
		{//only msg, display it
			//DisplayMessageRecv(mqid);
			
		}
		else
		{//others msgs being displayed - set Dismiss to Read Next
			SetCaptionToNext(uin);
		}
		
		
	}	
	//tell plugins
	Plugin_NotifyPlugins(PM_GOTMESSAGE,0,(WPARAM)&msgque[mqid]);

	if (rto.status == STATUS_ONLINE && (opts.flags1 & FG_POPUP)) SendMessage(ghwnd,WM_HOTKEY,HOTKEY_SETFOCUS,0);
}

void CbRecvUrl(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *url, const char *desc)
{
	int mqid=0;
	char buffer[2048];
	
	
	sprintf(buffer, "%s\r\n%s", url, desc);
	AddToHistory(uin, MakeTime(hour, minute, day, month, year), HISTORY_URLRECV, buffer);
	if (!(GetUserFlags(uin) & UF_IGNORE))
	{
		PlaySoundEvent("MICQ_RecvUrl");
		mqid=MsgQue_Add(uin,hour,minute,day,month,year,(char *)desc,(char *)url,TRUE);
		SetGotMsgIcon(uin,TRUE);

		if (MsgQue_MsgsForUIN(uin)==1)
		{//only msg, display it
			//DisplayUrlRecv(mqid);
			
		}
		else
		{//others msgs being displayed - set Dismiss to Read Next
			SetCaptionToNext(uin);
		}
	}

	//tell plugins
	Plugin_NotifyPlugins(PM_GOTURL,0,(WPARAM)&msgque[mqid]);

	if (rto.status == STATUS_ONLINE && (opts.flags1 & FG_POPUP)) SendMessage(ghwnd,WM_HOTKEY,HOTKEY_SETFOCUS,0);
}

void CbUserOnline(struct icq_link *link, unsigned long uin, unsigned long status, unsigned long ip, unsigned short port, unsigned long real_ip, unsigned char tcp_flag)
{	
	int i=0;
	PlaySoundEvent("MICQ_UserOnline");
	//if (status > 0xFFFF) status >>= 16;
	ChangeContactStatus(uin, status);
	for (i = 0; i < opts.ccount; i++)
	{
		if (uin==opts.clist[i].uin)
		{
			opts.clistrto[i].IP=ip;
			opts.clistrto[i].REALIP=real_ip;
			break;
		}
	}

	ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
}

void CbUserOffline(struct icq_link *link, unsigned long uin)
{
	PlaySoundEvent("MICQ_UserOffline");
	ChangeContactStatus(uin, STATUS_OFFLINE);

	ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
}

void CbUserStatusUpdate(struct icq_link *link, unsigned long uin, unsigned long status)
{
	ChangeContactStatus(uin, status);

	ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
}

void CbInfoReply(struct icq_link *link, unsigned long uin, const char *nick, const char *first, const char *last, const char *email, char auth)
{
	LVITEM item;
	LVFINDINFO findinfo;
	INFOREPLY inforeply;
	HWNDLIST *node;

	int i;
	int ri;
	// cha cha cha
	inforeply.uin = uin;
	inforeply.nick = (char *)nick;
	inforeply.first = (char *)first;
	inforeply.last = (char *)last;
	inforeply.email = (char *)email;

	node = rto.hwndlist;
	while (node)
	{
		if (node->windowtype == WT_DETAILS)
		{
			SendMessage(node->hwnd, TI_INFOREPLY, 0, (LPARAM)&inforeply);
		}
		node = node->next;
	}
	// update the info in the contact list
	for (i = 0; i < opts.ccount; i++)
	{
		if (opts.clist[i].uin == uin)
		{
			if (!*(opts.clist[i].first)) 
				strcpy(opts.clist[i].first, first);
			if (!*(opts.clist[i].last)) 
				strcpy(opts.clist[i].last, last);
			if (!*(opts.clist[i].email)) 
				strcpy(opts.clist[i].email, email);
			if (!*(opts.clist[i].nick)) 
				strcpy(opts.clist[i].nick, nick);
			ListView_SortItems(rto.hwndContact, lvSortFunc, ++rto.sortmode);
			ri=i;
		}
	}
	// update the info on the list view
	
	if (opts.clist[ri].nick)
	{
		findinfo.flags = LVFI_PARAM;
		findinfo.lParam = uin;
		i = ListView_FindItem(rto.hwndContact, -1, &findinfo);
			
		if (i != -1)
		{
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.pszText = (char *)opts.clist[ri].nick;
			item.iSubItem = 0;			
			ListView_SetItem(rto.hwndContact, &item);
		}
	}
}

void CbExtInfoReply(struct icq_link *link, unsigned long uin, const char *city, unsigned short country_code, char country_stat, const char *state, unsigned short age, char gender, const char *phone, const char *hp, const char *about)
{
	HWNDLIST *node;
	EXTINFOREPLY extinforeply;

	extinforeply.uin = uin;
	extinforeply.city = (char *)city;
	extinforeply.country_code = country_code;
	extinforeply.state = (char *)state;
	extinforeply.age = age;
	extinforeply.gender = gender;
	extinforeply.phone = (char *)phone;
	extinforeply.hp = (char *)hp;
	extinforeply.about = (char *)about;

	node = rto.hwndlist;
	while (node)
	{
		if (node->windowtype == WT_DETAILS)
		{
			SendMessage(node->hwnd, TI_EXTINFOREPLY, 0, (LPARAM)&extinforeply);
		}
		node = node->next;
	}
}

void CbSrvAck(struct icq_link *link, unsigned short seq)
{
	HWNDLIST *node;
	node = rto.hwndlist;

	//connectionAlive=450; // for alivecheck in timer function (wndproc)

	while (node)
	{
		if (seq == node->ack) 
		{
			SendMessage(node->hwnd, TI_SRVACK, 0, 0);
			return;
		}
		node = node->next;
	}
}

void CbRequestNotify(struct icq_link *link, unsigned long id, int result, unsigned int length, void *data)
{
	HWNDLIST *node;
	node = rto.hwndlist;

	//connectionAlive=520; // for alivecheck in timer function (wndproc)

	while (node)
	{
		if (id == node->ack) 
		{
			SendMessage(node->hwnd, TI_SRVACK, 0, 0);
			return;
		}
		node = node->next;
	}
}

void CbRecvAdded(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *nick, const char *first, const char *last, const char *email)
{
	HWNDLIST *node;
	DLG_DATA dlg_data;

	PlaySoundEvent("MICQ_Added");

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_ADDED;
	node->next = rto.hwndlist;
	node->uin = uin;
	rto.hwndlist = node;

	dlg_data.nick = (char *)nick;
	dlg_data.uin = uin;
	dlg_data.hour = hour;
	dlg_data.minute = minute;
	dlg_data.day = day;
	dlg_data.month = month;
	dlg_data.year = year;

	node->hwnd = CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_ADDED), NULL, DlgProcAdded, (LPARAM)&dlg_data);
	//ShowWindow(node->hwnd, (opts.flags1 & FG_POPUP) ? SW_SHOW : SW_SHOWMINIMIZED);
	ShowWindow(node->hwnd, SW_SHOW);
}

void CbRecvAuthReq(struct icq_link *link, unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *nick, const char *first, const char *last, const char *email, const char *reason)
{
	HWNDLIST *node;
	DLG_DATA dlg_data;

	PlaySoundEvent("MICQ_AuthReq");

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_AUTHREQ;
	node->next = rto.hwndlist;
	node->uin = uin;
	rto.hwndlist = node;

	dlg_data.msg = reason;
	dlg_data.nick = nick;
	dlg_data.uin = uin;
	dlg_data.hour = hour;
	dlg_data.minute = minute;
	dlg_data.day = day;
	dlg_data.month = month;
	dlg_data.year = year;

	node->hwnd = CreateDialogParam(ghInstance, MAKEINTRESOURCE(IDD_AUTHREQ), NULL, DlgProcAuthReq, (LPARAM)&dlg_data);

	ShowWindow(node->hwnd, SW_SHOW);
}

void CbUserFound(struct icq_link *link, unsigned long uin, const char *nick, const char *first, const char *last, const char *email, char auth)
{
	HWNDLIST *node;
	DLG_DATA dlg_data;

	dlg_data.uin = uin;
	dlg_data.nick = (char *)nick;
	dlg_data.first = (char *)first;
	dlg_data.last = (char *)last;

	node = rto.hwndlist;
	while (node)
	{
		if (node->windowtype == WT_RESULT)
		{
			SendMessage(node->hwnd, TI_USERFOUND, 0, (LPARAM)&dlg_data);
		}
		node = node->next;
	}	
}

void CbSearchDone(struct icq_link *link)
{
	HWNDLIST *node;

	node = rto.hwndlist;
	while (node)
	{
		if (node->windowtype == WT_RESULT)
		{
			SendMessage(node->hwnd, WM_SETTEXT, 0,  (LPARAM)"Search done.");
		}
		node = node->next;
	}
}

void CbWrongPassword(struct icq_link *link)
{
	PlaySoundEvent("MICQ_Error");
	MessageBox(ghwnd, "Your password is incorrect", MIRANDANAME, MB_OK);
}

void CbInvalidUIN(struct icq_link *link)
{
	PlaySoundEvent("MICQ_Error");
	MessageBox(ghwnd, "Miranda made a request with an invalid UIN", MIRANDANAME, MB_OK);
}

void CbLog(struct icq_link *link, time_t time, unsigned char level, const char *str)
{
	#ifdef _DEBUG
		OutputDebugString(str);
	#endif
	//send the msg to the plugins
	Plugin_NotifyPlugins(PM_ICQDEBUGMSG,(WPARAM)strlen(str),(LPARAM)str);
}

void CbRecvFileReq(struct icq_link *link, unsigned long uin,
       unsigned char hour, unsigned char minute, unsigned char day,
       unsigned char month, unsigned short year, const char *descr,
       const char *filename, unsigned long filesize, unsigned long seq)
{
	
	char msg[100];
	char *buf;
	buf=LookupContactNick(uin);
	if (buf)
		sprintf(msg,"%s is trying to send you a file, but this version of %s is unable to accept file.",buf,MIRANDANAME);
	else
		sprintf(msg,"%u is trying to send you a file, but this version of %s is unable to accept file.",uin,MIRANDANAME);

	MessageBox(ghwnd,msg,MIRANDANAME,MB_OK);
	
	
	/*HWNDLIST *node;
	DLG_DATA dlg_data;

	node = malloc(sizeof(HWNDNODE));

	node->windowtype = WT_RECVFILE;
	node->next = rto.hwndlist;
	node->uin = uin;
	rto.hwndlist = node;

	dlg_data.uin = uin;
	dlg_data.hour = hour;
	dlg_data.minute = minute;
	dlg_data.day = day;
	dlg_data.month = month;
	dlg_data.year = year;
	dlg_data.msg = descr;
	
	ShowWindow(CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_RECVFILE), ghwnd, DlgProcRecvFile,(LPARAM)&dlg_data), SW_SHOW);
	*/

}


// end: call back functions

int CALLBACK lvSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CONTACT *a;
	CONTACT *b;

	int i;

	for (i = 0; i < opts.ccount; i++)
	{
		if ((unsigned int)lParam1 == opts.clist[i].uin)
			a = &opts.clist[i];
		if ((unsigned int)lParam2 == opts.clist[i].uin)
			b = &opts.clist[i];
	}

	if ((a->status == 0xFFFF && b->status == 0xFFFF) ||
		(a->status != 0xFFFF && b->status != 0xFFFF))
	{
		return _stricmp(a->nick, b->nick);
	}
	else
	{
		if (a->status == 0xFFFF) return +1;
		if (b->status == 0xFFFF) return -1;
	}
	return 0;
}


void ShowHide()
{
	if (rto.hidden)
	{
		ShowWindow(ghwnd, SW_RESTORE);
		rto.hidden = FALSE;
		SetWindowPos(ghwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		if (!(opts.flags1 & FG_ONTOP)) SetWindowPos(ghwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetForegroundWindow(ghwnd);
	}
	else
	{
		if (opts.flags1 & FG_ONTOP)
		{
			ShowWindow(ghwnd, SW_HIDE);
			rto.hidden = TRUE;
		}
		else
		{
			RECT crect;
			RECT wrect;

			GetClipBox(GetDC(ghwnd), &crect);
			GetClientRect(ghwnd, &wrect);

			if (crect.bottom == wrect.bottom &&
				crect.top == wrect.top &&
				crect.left == wrect.left &&
				crect.right == wrect.right)
			{
				ShowWindow(ghwnd, SW_HIDE);
				rto.hidden = TRUE;				
			}
			else
			{
				SetWindowPos(ghwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				SetWindowPos(ghwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);			
			}
		}
	}
}


HWND CheckForDupWindow(unsigned long uin,long wintype)
{
	HWNDLIST *node;

	node = rto.hwndlist;
	if (node==NULL){return FALSE;}
	if (node->uin == uin && node->windowtype==wintype)
	{
		
	return node->hwnd;
		
	}
	else
	{
		while (node)
		{
			if (node->next==NULL){return FALSE;}
			if (uin == node->next->uin && node->next->windowtype==wintype)
			{
				return node->next->hwnd;
			}
			node = node->next;
		}
	}

	return NULL;

}

void ShowNextMsg(int mqid) //shows next msg if it exists , also removes old msg from msgque array
{//DONT rearrange, remove on or both fo the msgque_remove ,their positioning (mainly the top one) is important
	unsigned long uin;
	int newmqid;
	if (MsgQue_MsgsForUIN(msgque[mqid].uin)>1)
	{
		uin=msgque[mqid].uin;
		MsgQue_Remove(mqid);

		newmqid=MsgQue_FindFirst(uin);
		if (msgque[mqid].isurl)
		{//URL
			DisplayUrlRecv(newmqid);
		}
		else
		{//msg
			DisplayMessageRecv(newmqid);
		}
		
	}
	else
	{
		uin=msgque[mqid].uin;
		MsgQue_Remove(mqid); //remove must be bore setgotmsgicon func
		SetGotMsgIcon(uin,FALSE);
		
	}

}


void SetCaptionToNext(unsigned long uin) //sets the cancel btn to Next (used when a msg is qued up)
{
	HWND msgwnd;
	msgwnd=CheckForDupWindow(uin,WT_RECVMSG);
	if (msgwnd)
	{	
		SetDlgItemText(msgwnd, IDCANCEL, "&Next");
	}
	
	msgwnd=CheckForDupWindow(uin,WT_RECVURL);
	if (msgwnd)
	{	
		SetDlgItemText(msgwnd, IDCANCEL, "&Next");
	}			

}


void SetGotMsgIcon(unsigned long uin,BOOL gotmsg)
{

	LVITEM item;
	LVFINDINFO findinfo;
	int i;
	int idx;
	findinfo.flags = LVFI_PARAM;
	findinfo.lParam = uin;
	idx = ListView_FindItem(rto.hwndContact, -1, &findinfo);

	if (idx != -1)
	{
		item.mask = LVIF_IMAGE;
		item.iItem = idx;
		if (gotmsg)
		{
			item.iImage = 8; //IMAGE ID (9th pos -1)
		}
		else
		{
			for (i = 0; i < opts.ccount; i++)
			{
				if (uin == opts.clist[i].uin)
				{
					item.iImage=GetStatusIconIndex(opts.clist[i].status);
					break;
				}
			}
			
		}
		item.iSubItem = 0;			
		ListView_SetItem(rto.hwndContact, &item);
	}

	
	TrayIcon(ghwnd, TI_UPDATE);

}

void SendFiles(HWND hwnd, unsigned long uin)
{
	char buf[MAX_PATH];

	int cnt;
	char **files;

	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = ghInstance;
	ofn.lpstrFilter = "All Files\0*.*\0";
	ofn.lpstrFile = buf;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.nMaxFile = sizeof(buf);
	
	if (GetOpenFileName(&ofn))
	{
		files = calloc(20, sizeof(char*));
		if (GetFileAttributes(buf) == FILE_ATTRIBUTE_DIRECTORY)
		{
			char *p;

			cnt = 0;
			p = buf + strlen(buf) + 1;

			while (*p && cnt < 20)
			{
				files[cnt] = calloc(MAX_PATH, sizeof(char));

				strcpy(files[cnt], buf);
				strcat(files[cnt], "\\");
				strcat(files[cnt], p);
				p += strlen(p) + 1;
				cnt++;
			}
		}
		else
		{
			cnt = 1;
			files[0] = calloc(MAX_PATH, sizeof(char));
			strcpy(files[0], buf);
		}
		icq_SendFileRequest(plink, uin, "Here are some files", files);
	}
	{
		int i;
		i = GetLastError();
	}

}

void tcp_engage(void)
{
	WSADATA wsadata;

	WSAStartup(MAKEWORD(1,1),&wsadata);

	memset(&link, 0, sizeof(link));
	plink = &link;

	icq_LogLevel = ICQ_LOG_MESSAGE;
}

void tcp_disengage(void)
{	
	WSACleanup();
}

void IPtoString(unsigned long IP,char* strIP)
{//convert a 4 byte long IP into a string IP
	
	char tmp[15];
	sprintf(tmp,"%d.%d.%d.%d",((IP & 0xFF000000)>>24),((IP & 0x00FF0000)>>16),((IP & 0x0000FF00)>>8),(IP & 0x000000FF));
	strcpy(strIP,tmp);
}

void InitMenuIcons()
{
// customize menus
	HMENU hMenu;
	
	MENUITEMINFO lpmii;
	
	
	lpmii.fMask = MIIM_BITMAP | MIIM_DATA;
	lpmii.hbmpItem = HBMMENU_CALLBACK; //LoadBitmap(ghInstance,MAKEINTRESOURCE(IDB_OCCUPIED));
	lpmii.dwItemData=MFT_OWNERDRAW;//DRAW_ICONONLY;
	lpmii.cbSize = sizeof(lpmii);
	
	//STATUS MNU ICONS
	hMenu=GetSubMenu(GetMenu(ghwnd),1);
	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_OFFLINE,FALSE,&lpmii);

	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_ONLINE,FALSE,&lpmii);

	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_AWAY,FALSE,&lpmii);

	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_DND,FALSE,&lpmii);

	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_NA,FALSE,&lpmii);	
	
	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_OCCUPIED,FALSE,&lpmii);	

	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_FREECHAT,FALSE,&lpmii);	

	SetMenuItemInfo (hMenu,ID_ICQ_STATUS_INVISIBLE,FALSE,&lpmii);
	
	
	//ICQ MNU ICONS
	hMenu=GetSubMenu(GetMenu(ghwnd),0);
	
	//GIVE NAMES to mnu items that are popups (show a submenu)
	
	lpmii.fMask=MIIM_BITMAP | MIIM_ID | MIIM_DATA;
	lpmii.wID=ID_ICQ_ADDCONTACT;
	SetMenuItemInfo (hMenu,0,TRUE,&lpmii);
	
	lpmii.fMask=MIIM_BITMAP | MIIM_ID | MIIM_DATA;
	lpmii.wID=ID_ICQ_OPTIONS;
	SetMenuItemInfo (hMenu,1,TRUE,&lpmii);
	
	lpmii.fMask=MIIM_BITMAP | MIIM_ID | MIIM_DATA;
	lpmii.wID=ID_ICQ_OPTIONS_PLUGINS;
	SetMenuItemInfo(hMenu,2,TRUE,&lpmii);	

	lpmii.fMask = MIIM_BITMAP | MIIM_DATA; //put back to normal
	
	SetMenuItemInfo(hMenu,ID_ICQ_VIEWDETAILS,FALSE,&lpmii);

	//ADD Submenu (parent ICQ)
	hMenu=GetSubMenu(hMenu,0);
	SetMenuItemInfo(hMenu,ID_ICQ_ADDCONTACT_BYICQ,FALSE,&lpmii);
	SetMenuItemInfo(hMenu,ID_ICQ_ADDCONTACT_BYNAME,FALSE,&lpmii);
	SetMenuItemInfo(hMenu,ID_ICQ_ADDCONTACT_BYEMAIL,FALSE,&lpmii);
	SetMenuItemInfo(hMenu,ID_ICQ_ADDCONTACT_IMPORT,FALSE,&lpmii);

	//OPS Submenu (parent ICQ)
	hMenu=GetSubMenu(GetMenu(ghwnd),0);
	hMenu=GetSubMenu(hMenu,1);
	SetMenuItemInfo(hMenu,ID_ICQ_OPTIONS_SOUNDOPTIONS,FALSE,&lpmii);
	//SetMenuItemInfo(hMenu,ID_ICQ_OPTIONS_PROXYSETTINGS,FALSE,&lpmii);
	SetMenuItemInfo(hMenu,ID_ICQ_OPTIONS_GENERALOPTIONS,FALSE,&lpmii);
	SetMenuItemInfo(hMenu,ID_ICQ_OPTIONS_ICQPASSWORD,FALSE,&lpmii);
	
	//HLP MNU ICONS
	hMenu=GetSubMenu(GetMenu(ghwnd),2);
	//SetMenuItemInfo (hMenu,ID_HELP_ABOUT,FALSE,&lpmii);
	SetMenuItemInfo (hMenu,ID_HELP_CKECKFORUPGRADES,FALSE,&lpmii);
	SetMenuItemInfo (hMenu,ID_HELP_WINDEX,FALSE,&lpmii);
	
	SetMenuItemInfo (hMenu,ID_HELP_MIRANDAWEBSITE,FALSE,&lpmii);

}

int GetMenuIconNo(long ID, BOOL *isCheck)
{
int i;
	switch (ID)
	{
	case ID_ICQ_STATUS_ONLINE:
		i=GetStatusIconIndex(STATUS_ONLINE);
		*isCheck=TRUE;
		break;
	case ID_ICQ_STATUS_OFFLINE:
		i=GetStatusIconIndex(STATUS_OFFLINE);
		*isCheck=TRUE;
		break;
	case ID_ICQ_STATUS_OCCUPIED:
		i=GetStatusIconIndex(STATUS_OCCUPIED);
		*isCheck=TRUE;
		break;
	case ID_ICQ_STATUS_AWAY:
		i=GetStatusIconIndex(STATUS_AWAY);
		*isCheck=TRUE;
		break;
	case ID_ICQ_STATUS_INVISIBLE:
		i=GetStatusIconIndex(STATUS_INVISIBLE);
		*isCheck=TRUE;
		break;
	case ID_ICQ_STATUS_NA:
		i=GetStatusIconIndex(STATUS_NA);
		*isCheck=TRUE;
		break;
	case ID_ICQ_STATUS_DND:
		i=GetStatusIconIndex(STATUS_DND);
		*isCheck=TRUE;
		break;
	case ID_ICQ_STATUS_FREECHAT:
		i=GetStatusIconIndex(STATUS_FREE_CHAT);
		*isCheck=TRUE;
		break;
	case ID_ICQ_OPTIONS:
		i=10;
		break;
	case ID_ICQ_ADDCONTACT:
	case ID_ICQ_ADDCONTACT_BYICQ:
	case ID_ICQ_ADDCONTACT_BYEMAIL:
	case ID_ICQ_ADDCONTACT_BYNAME:
		i=12;
		break;
	case ID_ICQ_VIEWDETAILS:
		i=11;
		break;
	case ID_HELP_WINDEX: 
	case ID_HELP_ABOUT:
		i=13;
		break;
	case ID_HELP_CKECKFORUPGRADES:
		i=14;
		break;
	//case ID_ICQ_OPTIONS_PROXYSETTINGS:
	//	i=15;
	//	break;
	case ID_ICQ_OPTIONS_SOUNDOPTIONS:
		i=16;
		break;	
	case ID_ICQ_OPTIONS_PLUGINS:
		i=17;
		break;
	case ID_ICQ_OPTIONS_GENERALOPTIONS:
		i=18;
		break;
	case ID_ICQ_OPTIONS_ICQPASSWORD:
		i=19;
		break;
	case ID_HELP_MIRANDAWEBSITE:
		i=20;
		break;
	case POPUP_SENDMSG: //USER POPUP MENU ICONS
		i=21;
		break;
	case POPUP_SENDURL:
		i=22;
		break;
	case POPUP_HISTORY:
		i=23;
		break;
	case POPUP_IGNORE:
		i=26;
		*isCheck=TRUE;
		break;
	case POPUP_USERDETAILS:
		i=11;
		
		break;
	case POPUP_RENAME:
		i=24;
		break;
	case POPUP_DELETE:
		i=25;
		break;
	case ID_ICQ_ADDCONTACT_IMPORT:
		i=27;
		break;
	/*case ID_POPUP_SENDURL:
		i=17;
		break;		
	case ID_POPUP_SENDMESSAGE:
		i=12;
		break;
	case ID_POPUP_IGNOREUSER:
		i=14;
		*isCheck=TRUE;
		break;
	case ID_POPUP_HISTORY:
		i=13;
		break;
	case ID_POPUP_DELETE:
		i=8;
		break;
	case ID_POPUP_RENAME:
		i=15;
		break;
	
	case ID_POPUP_USERDETAILS:
	case ID_ICQ_VIEWDETAILS:
		i=10;
		break;
	case ID_ICQ_STATUS:
		i=GetStatusIconIndex(rto.desiredStatus);
		break;
	
	
	default:
		i=10;
		break;*/
	}
	return i;
}

void parseCmdLine(char *p)
{
	char *token;

	strcpy(myprofile, "default");
	//GetCurrentDirectory(sizeof(mydir), mydir);
	GetModuleFileName(ghInstance, mydir, sizeof(mydir));
	*strrchr(mydir, '\\') = 0;
	strcat(mydir, "\\");

	token = strtok(p, " \t");
	while (token)
	{
		if (token[0] == '-' || token[0] == '/')
		{
			switch (token[1])
			{
				case 'd':
				case 'D':
					token += 2;
					memset(mydir, 0, sizeof(mydir));
					strcpy(mydir, token);
					if (mydir[strlen(mydir)-1]!='\\')
					{
						mydir[strlen(mydir)]   = '\\';
						mydir[strlen(mydir)+1] = '\0';
					}
					SetCurrentDirectory(mydir);
					break;
			}
		}
		else
		{
			strcpy(myprofile, token);
		}
		token = strtok(NULL, " \t");
	}
	free(p);
}
