
#include "msgque.h"
#include "malloc.h"
#include <windows.h>

MESSAGE *msgque;

int msgquecnt=0;



int MsgQue_Add(unsigned long puin,unsigned char phour, unsigned char pminute, unsigned char pday,unsigned char pmonth,unsigned short pyear, char *pmsg,char*purl,BOOL isurl)
{
	int len;
	if (msgquecnt==0)
	{
		msgque=(MESSAGE*)malloc(sizeof(MESSAGE));
	}
	else
	{
		msgque=(MESSAGE*)realloc(msgque,(msgquecnt+1)*sizeof(MESSAGE));
	}

	msgque[msgquecnt].day=pday;
	msgque[msgquecnt].month=pmonth;
	msgque[msgquecnt].year=pyear;
	msgque[msgquecnt].hour=phour;
	msgque[msgquecnt].minute=pminute;
	msgque[msgquecnt].beingread=FALSE;


	len=strlen(pmsg)+1;
	msgque[msgquecnt].msg=(char*)malloc(len);
	strcpy(msgque[msgquecnt].msg,pmsg);
	
	if (isurl) 
	{
		msgque[msgquecnt].url=(char*)malloc(strlen(purl));
		strcpy(msgque[msgquecnt].url,purl);
	}
	
	msgque[msgquecnt].isurl=isurl;
	msgque[msgquecnt].uin=puin;
	
	
	msgquecnt++;

	return (msgquecnt-1);
}


int MsgQue_MsgsForUIN(unsigned long uin)//,BOOL url)
{
	int i=0;
	int cnt=0;
	for(i=0;i<msgquecnt;i++)
	{
		if (msgque[i].uin==uin)// && msgque[i].isurl==url )
		{
			cnt++;
		}

	}
	return cnt;
}


void MsgQue_Remove(int id)
{
	int i;
	for (i=id;i<msgquecnt-1;i++)
	{
		msgque[i].uin=msgque[i+1].uin;
		msgque[i].day=msgque[i+1].day;
		msgque[i].month=msgque[i+1].month;
		msgque[i].year=msgque[i+1].year;
		msgque[i].hour=msgque[i+1].hour;
		msgque[i].minute=msgque[i+1].minute;
		msgque[i].beingread=msgque[i+1].beingread;	
	
		msgque[i].msg=(char*)realloc(msgque[i].msg,strlen(msgque[i+1].msg)+1);
		if (!msgque[i+1].isurl)
		{
			if(msgque[i].isurl)
			{
				free(msgque[i].url);
			}
		}
		else
		{
			if (msgque[i].isurl)
			{
				msgque[i].url=(char*)realloc(msgque[i].url,strlen(msgque[i+1].url)+1);
			}
			else
			{
				msgque[i].url=(char*)malloc(strlen(msgque[i+1].url)+1);
			}
		}

		strcpy(msgque[i].msg,msgque[i+1].msg);
		if (msgque[i+1].isurl)
		{
			strcpy(msgque[i].url,msgque[i+1].url);
		}
		msgque[i].isurl=msgque[i+1].isurl; //this placing is important (cant be before the IF)
	}

	if (msgquecnt==1)//really 0 when were finished
	{
		free(msgque);
	}
	else
	{
		msgque=(MESSAGE*)realloc(msgque,(msgquecnt-1)*sizeof(MESSAGE));
	}
	
	msgquecnt--;
}


int MsgQue_FindFirst(unsigned long uin)
{
	int i=0;
	
	for(i=0;i<msgquecnt;i++)
	{
		if (msgque[i].uin==uin)// && msgque[i].isurl==url)
		{
			return i;
		}

	}
	return -1;

}


int MsgQue_FindFirstUnread()
{
	int i=0;
	for (i=0;i<msgquecnt;i++)
	{
		if (msgque[i].beingread==FALSE)
		{
			return i;
		}
	}
	return -1;

}

int MsgQue_ReadCount()
{
	int cnt=0;
	int i=0;
	for (i=0;i<msgquecnt;i++)
	{
		if (msgque[i].beingread==TRUE)
		{
			cnt++;
		}
	}
	return cnt;
}