#ifndef _COMMON_H_
#define _COMMON_H_

#define MAXLEN_CMD 100
typedef unsigned char uchar;

void DieWithUserMessage(const char *, const char *);
void DieWithSystemMessage(const char *);
uchar*trimLeft(uchar*);
uchar*trimRight(uchar*);
int numOfParametersCheck(const uchar *,const int );
int CmdFormatCheck(const uchar *);
#endif