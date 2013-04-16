#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLEN_CMD 100
typedef unsigned char uchar;
void DieWithUserMessage(const char *msg, const char *detail) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  exit(1);
}
void DieWithSystemMessage(const char *msg) {
  perror(msg);
  exit(1);
}

uchar *trimLeft(uchar *str)
{
    if(str==NULL || strlen(str)==0)
        return NULL;
    for(;*str==0x20 || *str=='\t'; ++str);
    return str;
}
uchar *trimRight(uchar *str)
{
	uchar *strTmp = str+strlen(str)-1;
    if(str==NULL || strlen(trimLeft(str))==0)
		return NULL;
	while ((*strTmp == ' ' || *strTmp == '\t' || *strTmp == 10) && strTmp>str)
	{
		*strTmp = '\0';
		strTmp--;
	}
	return str;
}
int numOfParametersCheck(const uchar *str,const int num)
{
    int numParameters=0;
	uchar cpyStr[MAXLEN_CMD];
	strcpy(cpyStr,str);
	strtok(cpyStr," ");
	while(strtok(NULL," ")!=NULL)
	{
	    numParameters++;
	    if(numParameters>num)
	    {
	        printf("Cmd should have %d parameter!\n\n",num);
            return 0;
	    }
	}
	if(numParameters==num)
	{
	    return 1;
	}
	else  //numParameters<num
	{
        printf("Cmd should have %d parameter!\n\n",num);
	    return 0;
	}
}
int CmdFormatCheck(const uchar *str)
{
	if(strlen(str)>=90)
	{
        return -1;
	}
	if(strncasecmp("Register",str,8)==0)
	{
        if(numOfParametersCheck(str, 2)==1)
        {
            return 1;
        }
	}
	if(strncasecmp("Login",str,5)==0 )
	{
        if(numOfParametersCheck(str, 2)==1)
        {
            return 2;
        }
	}
	if(strncasecmp("ModifyPwd",str,9)==0)
	{
        if(numOfParametersCheck(str, 2)==1)
        {
            return 3;
        }
	}

    if(strncasecmp("Download",str,8)==0 )
    {
        if(numOfParametersCheck(str, 1)==1)
        {
            return 4;
        }
    }
    if(strncasecmp("Delete",str,6)==0)
    {
        if(numOfParametersCheck(str, 1)==1)
        {
            return 5;
        }
    }
	if(strncasecmp("Synchronize",str,11)==0 )
	{
	    return 6;
	}
	if(strncasecmp("List",str,4)==0 )
	{
	    return 7;
	}
	if(strncasecmp("help",str,4)==0 )
	{
	    return 8;
	}
	if(strncasecmp("quit",str,4)==0)
	{
	    return 9;
	}
	if(strncasecmp("send",str,4)==0)
	{
	    return 10;
	}
	if(strncasecmp("clear",str,5)==0)
	{
	    return 11;
	}
	if(strncasecmp("clsRecords",str,10)==0)
	{
	    return 12;
	}
	if(strncasecmp("recv",str,4)==0)
	{
	    return 13;
	}
	if(strncasecmp("show",str,4)==0)
	{
	    return 14;
	}
	return 0;

}

int serverCmdCheck(const uchar *str)
{
	if(strncasecmp("Register",str,8)==0)             return 1;
	if(strncasecmp("Login",str,5)==0 )                 return 2;
	if(strncasecmp("ModifyPwd",str,9)==0)        return 3;
    if(strncasecmp("Download",str,8)==0 )         return 4;
    if(strncasecmp("Delete",str,6)==0)                return 5;
	if(strncasecmp("Synchronize",str,11)==0 )  return 6;
	if(strncasecmp("List",str,4)==0 )                    return 7;
	if(strncasecmp("quit",str,4)==0)              	     return 8;
	if(strncasecmp("send",str,4)==0)              	 return 9;
	return 0;

}

