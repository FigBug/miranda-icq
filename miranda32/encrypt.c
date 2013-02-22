
#include "encryption.h"
//VERY VERY VERY BASIC ENCRYPTION FUNCTION

void Encrypt(char*msg,BOOL up)
{
	int i;
	int jump;
	if (up==TRUE)
	{
		jump=5;
	}
	else
	{
		jump=-5;
	}

	for (i=0;i<strlen(msg);i++)
	{
			msg[i]=msg[i]+jump;
	}

}