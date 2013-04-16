#include "Common.h"
#define MAXLEN_CMD 100
#define BUFSIZE 100
typedef struct user
{
	uchar userName[10];
	uchar userPassword[10];
	struct user *next;
}Node, *LinkedList;

int nameDupCheck(char *);
int Register(uchar *) ;
int Login(uchar *);
int ModifyPwd(uchar *);
void fileRecv(uchar *, int );
void fileSend(uchar *, int );
void List(int );
int Delete(uchar *);
void Quit(int );
