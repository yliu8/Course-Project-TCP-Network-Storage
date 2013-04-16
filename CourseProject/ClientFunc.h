#include "Common.h"
#define MAXLEN_CMD 100
#define BUFSIZE 100

typedef struct info
{
	char fileName[20];
	int fileSize;
	int lastModifyTime;
} fileInfo;

typedef struct linkList
{
    fileInfo inf;
    fileInfo *next;
} finfNode, *finfLinklist;

char *getIPbyHostName(char *, char *);
void welcome();
void Register();
int beforeLogged(const uchar *);
void helpInfo();
void Quit(int );
void clear();
void Synchronize( int );
int SendFile(const uchar *, int );
void fileDownload(uchar *, int );
void Download(const uchar *, int );
void storeInfoIntoFile();
void generateLinklist(finfLinklist );
int fileUpload(char *,int , int );
void clsRecords();
