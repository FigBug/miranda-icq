
#include "../icqlib/icq.h"
#include "global.h"
#include "plugins.h"
#include "pluginapi.h"

extern ICQLINK *plink;
extern OPTIONS	opts;
extern OPTIONS_RT rto;

void ChangeStatus(HWND hWnd, int newStatus);

tagPlugin Plugin[MAX_PLUGINS];
int plugincnt=0;

extern HWND				ghwnd; //from miranda.c
extern char				mydir[MAX_PATH]; //the dir we are operating from (look for files in etc).

void pcbSendMessage(int uin, char *msg)
{
	icq_SendMessage(plink, uin, msg, ICQ_SEND_THRUSERVER);
}

void pcbSendURL(int uin, char *desc, char *url)
{
	icq_SendURL(plink, uin, url, desc, ICQ_SEND_THRUSERVER);
}

CONTACT* pcbGetContactFromUIN(int uin)
{
	int i;

	for (i = 0; i < opts.ccount; i++)
	{
		if (opts.clist[i].uin == (unsigned int)uin) return &opts.clist[i];
	}
	return NULL;
}

void pcbChangeStatus(int status)
{
	ChangeStatus(NULL, status);
}

int pcbGetStatus(void)
{
	return rto.status;
}

void Plugin_Load()
{

	WIN32_FIND_DATA ffdata;
	
	HANDLE ff;
	char dir[MAX_PATH];
	char curfile[MAX_PATH];
	char wildcard[MAX_PATH];
	
	
	Plugin_GetDir(dir);
	strcpy(wildcard,dir);
	strcat(wildcard,"*.dll");


	if ((ff=FindFirstFile(wildcard,&ffdata))==INVALID_HANDLE_VALUE)
	{ //error
		//MessageBox(NULL,"Can't Find Plugins",szTitle,MB_OK);
		return;
	}
	goto loadfile;
	for(;;)
	{	
		if ((FindNextFile(ff,&ffdata))==0)
		{	//error
			if (GetLastError()==ERROR_NO_MORE_FILES)
			{
				break;
			}
		}
	loadfile:
		strcpy(curfile,dir);
		strcat(curfile,ffdata.cFileName);
		if ((Plugin[plugincnt].hinst=LoadLibrary(curfile))!=NULL)
		{ //loaded ok, get proc addresses
			Plugin[plugincnt].loadfunc=(FARPROC_LOAD)GetProcAddress(Plugin[plugincnt].hinst,"Load");
			Plugin[plugincnt].closefunc=(FARPROC_CLOSE)GetProcAddress(Plugin[plugincnt].hinst,"Unload");
			Plugin[plugincnt].notifyfunc=(FARPROC_NOTIFY)GetProcAddress(Plugin[plugincnt].hinst,"Notify");
			
			
			plugincnt++;
				
		}
		
	}

	FindClose(ff);
}

/*returns <app path>\plugins\ */

void Plugin_GetDir(char*plugpath)
{
	char path[260];
	strcpy(path, mydir);
	if (path[strlen(path)-1]!='\\')
	{ // add '\'
		strcat(path,"\\");
	}
	strcat(path,"plugins\\");
	strcpy(plugpath,path);
}



void Plugin_LoadPlugins(HWND hwnd,HINSTANCE hinstance)
{
	PI_CALLBACK pi_cb;
	HMENU hmen;
	int i;

	pi_cb.pChangeStatus = pcbChangeStatus;
	pi_cb.pGetContact   = pcbGetContactFromUIN;
	pi_cb.pGetStatus    = pcbGetStatus;
	pi_cb.pSendMessage  = pcbSendMessage;
	pi_cb.pSendURL	   = pcbSendURL;

	if (plugincnt)
	{
		HMENU hmen;
		hmen = GetMenu(ghwnd);
		hmen = GetSubMenu(hmen, 0);
		hmen = GetSubMenu(hmen, 2);

		RemoveMenu(hmen, 0, MF_BYPOSITION);
	}

	for (i=0;i<plugincnt;i++)
	{
		if (Plugin[i].loadfunc==NULL) 
		{//dll isnt working prop (didnt export a Load func)
			goto dlldidntload;
		}
		if ((*Plugin[i].loadfunc)(hwnd,hinstance,Plugin[i].Title)==FALSE)
		{
			//failed to load, kill the DLL
dlldidntload:
			FreeLibrary(Plugin[i].hinst);
			Plugin[i].hinst=NULL;
		}
		else
		{ //DLL loaded ok, add to menu (for options call)
			
			hmen=GetMenu(ghwnd);
			hmen=GetSubMenu(hmen,0);
			
			//hmen=GetSubMenu(hmen,1);
			//hmen=GetSubMenu(hmen,5);
			hmen=GetSubMenu(hmen,2);
			AppendMenu(hmen,MF_STRING,IDC_PLUGMNUBASE+i,Plugin[i].Title);
		}		
	}
	Plugin_NotifyPlugins(PM_REGCALLBACK, (WPARAM)&pi_cb, 0);
}
void Plugin_ClosePlugins()
{
	int i;
	
	__try
	{
		for (i=0;i<plugincnt;i++)
		{
			if (Plugin[i].hinst!=NULL)
			{
				(*Plugin[i].closefunc)();
				//free the lib
				FreeLibrary(Plugin[i].hinst);
			}

		}
	}
	__except (1)
	{
		MessageBox(ghwnd, "Uh oh. One of the plugins crashed", "Uh oh", MB_OK);
	}
}
void Plugin_NotifyPlugins(long msg,WPARAM wParam,LPARAM lParam)
{
	int i;

	__try
	{
		for (i=0;i<plugincnt;i++)
		{
			if (Plugin[i].hinst!=NULL)
				(*Plugin[i].notifyfunc)(msg,wParam,lParam);
		}
	}
	__except (1)
	{
		MessageBox(ghwnd, "Uh oh. One of the plugins crashed", "Uh oh", MB_OK);
	}
}

void Plugin_NotifyPlugin(int id,long msg,WPARAM wParam,LPARAM lParam)
{
	__try
	{
		if (Plugin[id].hinst!=NULL)
			(*Plugin[id].notifyfunc)(msg,wParam,lParam);
	}
	__except (1)
	{
		MessageBox(ghwnd, "Uh oh. One of the plugins crashed", "Uh oh", MB_OK);
	}
}
