#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char *StringToUpper(char *upperSting) {
	int length, i;
	length = strlen(upperSting);
	for (i=0; i<length; i++)
	{
		upperSting[i] = toupper(upperSting[i]);
	}
	return upperSting;
}
char *StringToLower(char *lowerSting) {
	int length, i;
	length = strlen(lowerSting);
	for (i=0; i<length; i++)
	{
		lowerSting[i] = tolower(lowerSting[i]);
	}
	return lowerSting;
}
void StreamCpyWith0(char *strDes,const char * strSrc,  unsigned char size)
{
    int i;
    for(i=0; i<size; i++)
    {
        strDes[i]=strSrc[i];
    }
}
char *trimLeft(char *str)
{
    if(str==NULL || strlen(str)==0)
        return NULL;
    for(;*str==0x20 || *str=='\t'; ++str);
    return str;
}
char *trimRight(char *str)
{
	char *strTmp = str+strlen(str)-1;
    if(str==NULL || strlen(trimLeft(str))==0)
		return NULL;
	while ((*strTmp == ' ' || *strTmp == '\t' || *strTmp == 10) && strTmp>str)
	{
		*strTmp = '\0';
		strTmp--;
	}
	return str;
}

char *ReturnFirstString(char *str){
	int i=0;
	str=trimLeft(str);
	if(strlen(str)==0)
		return NULL;
	while(str[i]!=' ' && str[i]!='\t')
	{
		i++;
	}
	str[i]='\0';
	return str;
}
char *ModifyString(char *str){
	int i=0;
	str=trimLeft(str);
	if(strlen(str)==0)
		return NULL;
	while(str[i]!='\0' )
	{
		i++;
	}
	str[i]=' ';
	return str;
}
int CommandCheckAndReturncmdCode(char *str)
{
	int cmdCode=0;
	str=trimLeft(str);//key=login|add|remove|list|quit|[error]
	//puts(str);
	if(strncasecmp("login",str,5)==0)
		cmdCode=1;
	if(strncasecmp("add",str,3)==0)
		cmdCode=2;
	if(strncasecmp("remove",str,6)==0)
		cmdCode=3;
	if(strncasecmp("list",str,4)==0)
		cmdCode=4;
	if(strncasecmp("quit",str,4)==0)
		cmdCode=5;
	//printf("common cmdcode %d\n",cmdCode);
	return cmdCode;
}
int GetParaNumOfCmd(char *str)
{
	int num=1;
	int i=0;
	int flag=0;
	str=trimLeft(str);

	if(str==NULL || strlen(str)==0)
		return 0;

	for(i=0;i<strlen(str);i++)
	{
		if((str[i]!=' ' && str[i]!='\t') && flag==1)
		{
			num++;
			flag=0;
		}
		else if(str[i]==' ' ||str[i]=='\t')
		{
			flag=1;
		}
	}
	return num;
}
char *GetCmdParaStringWithcmdCode(int cmdCode ,char *str)
{
	str=trimLeft(str);
	if(cmdCode==1)
		str=trimLeft(str+6);
	if(cmdCode==2)
		str=trimLeft(str+3);
	if(cmdCode==3)
		str=trimLeft(str+7);
	if(cmdCode==4|| cmdCode==5)
		str=trimLeft(str+5);
	return str;
}
char *GetCmdParaString(char *str)
{
	//puts("0");
	if(GetParaNumOfCmd(str)==1)
		return NULL;
	//puts("1");
	str=trimLeft(str);
	for(;*str!=0x20 && *str!='\t' ; ++str);
	str=trimLeft(str);
	//puts("2");
	return str;
}
