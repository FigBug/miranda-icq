//ICQ DB IMPORTER - By Tristan Van de Vreede
//only been tested with icq 2000b .dat files, still has some problems

#include <windows.h>
#include <stdio.h>


#include "import.h"
#include "global.h"


//from miranda.c
extern HWND					ghwnd;
extern HINSTANCE				ghInstance;
extern void AddToContactList(unsigned int uin);
//////////////////

void ImportContacts(char* src)
{
	FILE *f;
	char c[1];
	unsigned long UIN;
	char un[6];
	int i;
	BOOL match;
	unsigned long *UINS;
	int uincnt=0;

	f=fopen(src,"rb");
	if (f)
	{
		for(;;)
		{
			if(feof(f)){break;}
			fgets(c,3,f);
			
			if (c[0]==0x4E &&  c[1]==0)
			{
				fgets(c,2,f);
				//cnt++;
				if (c[0]==0x069)
				{
					fgets(un,6,f);
					
					match=FALSE;

					UIN=(unsigned long)((un[3] & 0xFF)<<24) | ((un[2] & 0xFF)<<16) | ((un[1] & 0xFF)<<8) | (un[0] & 0xFF);
					
					
					for (i=0;i<uincnt;i++)
					{
						if (UIN==UINS[i])
						{ //match
							match=TRUE;
							break;
						}
					}

					if (match==FALSE)
					{
						
						if (uincnt==0)
						{
							UINS=(unsigned long*)malloc(sizeof(unsigned long));
						}
						else
						{
							UINS=(unsigned long*)realloc(UINS,sizeof(unsigned long)*(uincnt+1));
						}

						UINS[uincnt]=UIN;
						AddToContactList(UIN);
						uincnt++;
					}

					//}


				}
			}


		}
		free(UINS);
		fclose(f);
	}
	

	if (uincnt==0)
	{
		MessageBox(ghwnd,"No contacts were found.",MIRANDANAME,MB_OK);
	}
	else
	{
		MessageBox(ghwnd,"Import complete. You must be online to get the user's details.",MIRANDANAME,MB_OK);
	}
}

void ImportPrompt()
{
	char buf[MAX_PATH] = {0};
	OPENFILENAME ofn;

	MessageBox(ghwnd,"Note: The ICQ DB Importer is still in its early stages and may have some problems.",MIRANDANAME,MB_OK);

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = ghwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "ICQ .dat file\0*.DAT\0";
	ofn.lpstrFile = buf;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.nMaxFile = sizeof(buf);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "dat";
	
	if (GetOpenFileName(&ofn))
		ImportContacts(buf);
}