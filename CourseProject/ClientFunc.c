#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <netdb.h>		//provide the function of gethostbyname()
#include "Common.h"
#include "wrapsock.h"
#define BUFSIZE 100
#define BUFFER_SIZE 190

int fileUpload(char *,int , int );
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
//Function: get Server IP using the Server Name
char *getIPbyHostName(char *servName, char *addr)
{
	struct hostent *host;
	char **p;
	if ((host=gethostbyname(servName))==NULL)
		DieWithUserMessage("gethostbyname()", "host connection failed");
	p=host->h_addr_list;
	inet_ntop(AF_INET, *p, addr, INET_ADDRSTRLEN);//Just get the first IP address of address list
	return addr;
}
void welcome()
{
    	system("clear");
    	fputs("Hello! Welcome to use TCP Network Storage Service. Now you can do:\n", stdout);
	fputs("  1) If you have no authorized account, then just Register one for yourself as (Register YourName YourPassword);\n", stdout);
	fputs("  2) Otherwise, LOGIN with your account as (Login YourName YourPassword);\n", stdout);
	fputs("  3) For more infomation, just input HELP.\n\n", stdout);
}
void helpInfo()
{
	printf("\t----help information----\n");
	printf("\t 1. Register userName passWord\n");
	printf("\t 2. Login userName passWord\n");
	printf("\t 3. ModifyPwd oldPassword newPassword\n");
	printf("\t 4. Synchronize\n");
	printf("\t 5. List\n");
	printf("\t 6. Download fileName\n");
	printf("\t 7. Delete fileName\n");
	printf("\t 8. Help\n");
	printf("\t 9. Quit\n");
	printf("\t10. Send fileName\n");
	printf("\t11. clear(clear the screen)\n");
	printf("\t12. clsRecords(clear local records, just try)\n");
	printf("\t13. show(a list of frame statistics)\n");
	//printf("\tAddutional command:set etc #\n");
	printf("----------------------------------------------------------\n");
}
//create .fileInfo.dat in the folder, which is a hidden configure file
void Register()
{
	FILE *fp;
	if((fp=fopen(".fileInfo.dat","w"))==NULL)
	{
		printf("File Open Error!\n");
		exit(1);
	}
	fclose(fp);
}
//check command before a user logged
int beforeLogged(const uchar *str)
{
	int cmdCode=CmdFormatCheck(str);
	if(cmdCode==-1)
	{
		printf("Too long command. Please re-input again:\n");
		return 0;
	}
    if(cmdCode!=1 && cmdCode!=2 )
    {
        if(cmdCode==8)
        {
            helpInfo();
            fputs("Now, input your command below:\n", stdout);
        }
        else
        {
            fputs("\nOnly Register, Login, Help commands work before you logged.\n", stdout);
            fputs("Please input your command again!\n", stdout);
        }
        return 0;
    }
    else
    {
        if(cmdCode==1)
        {
            Register();
        }
        return 1;
    }

}

// close socket and exit
int Quit(int sockfd)
{
	close(sockfd);
    exit(0);
}
//clear the screen
void clear()
{
    system("clear");
}
//use infomation of fileInfo.dat to generate a linked list ,
//which contains file name, file size and file last modified time, and will be used in "Syncheonize"
void generateLinklist(finfLinklist lklist)
{
    FILE *fp;
    finfLinklist lkl;
    fileInfo *fInfo=(fileInfo *)malloc(sizeof(fileInfo));

    lkl=lklist;
    if((fp=fopen(".fileInfo.dat","r"))==NULL)
	{
		printf("File Open Error2!\n");
		exit(1);
	}
    while(fread(fInfo,sizeof(struct info),1,fp)!=0)
	{
	    finfNode *fNode=(finfNode *)malloc(sizeof(finfNode));
	    strcpy((fNode->inf).fileName,fInfo->fileName);
	    (fNode->inf).fileSize=fInfo->fileSize;
	    (fNode->inf).lastModifyTime=fInfo->lastModifyTime;
		fNode->next=NULL;
        lkl->next=fNode;
        lkl=lkl->next;
	}
	fclose(fp);
}
//after each "Synchronize", update the .fileInfo.dat file
void storeInfoIntoFile()
{
	DIR *d;
	fileInfo *fInfo;
	struct dirent *file;
	struct stat buf;
	FILE *fp;
	char tempContainer[20];
	fInfo=(fileInfo *)malloc(sizeof(fileInfo));
	if((fp=fopen(".fileInfo.dat","w"))==NULL)
	{
		printf("File Open Error!\n");
		exit(1);
	}

	if(!(d = opendir(".")))
	{
		printf("error opendir!!!\n");
		exit(1);
	}
	chdir(".");
	while((file = readdir(d)) != NULL)
	{
		if(strncmp(file->d_name, ".", 1) == 0)
            continue;

		if(stat(file->d_name, &buf) >= 0 && !S_ISDIR(buf.st_mode) )
		{
			bzero(fInfo->fileName,20);
			strcpy(fInfo->fileName,file->d_name);
			fInfo->fileSize=buf.st_size;
			fInfo->lastModifyTime=buf.st_mtime;
			fwrite(fInfo,sizeof(struct info),1,fp);
		}
	}
	closedir(d);
	fclose(fp);
}

//batch upload file to the server side if one file is modified after last "Synchronize"
void Synchronize( int socket)
{
	DIR *d;
	struct dirent *file;
	struct stat buf;
	FILE *fp;
	finfLinklist lkl;
	int flag=0;
	uchar buffer[BUFSIZE];
	bzero(buffer,BUFSIZE);
	finfLinklist lklist=(finfNode *)malloc(sizeof(finfNode));
	lklist->next = NULL;
	generateLinklist(lklist);//create linklist
	system("clear");
    lkl=lklist->next;

	if((fp=fopen(".fileInfo.dat","w"))==NULL)
	{
		printf("File Open Error1!\n");
		exit(1);
	}
	if(!(d = opendir(".")))
	{
		printf("error opendir!!!\n");
		exit(1);
	}
	chdir(".");
	while((file = readdir(d)) != NULL)
	{
	    flag=0;
		if(strncmp(file->d_name, ".", 1) == 0 || strcmp(file->d_name,"Client")==0)//ignore hidden file and Client executable file
		{
		    continue;
		}
		if(stat(file->d_name, &buf) >= 0 && !S_ISDIR(buf.st_mode) )
		{
		    lkl=lklist->next;
		    while(lkl!= NULL )
		    {
                if(strcmp(file->d_name,(lkl->inf).fileName)==0)
                {
                    if(buf.st_mtime==(lkl->inf).lastModifyTime)
                    {
                            flag=1;
                            break;
                    }
                    else
                    {
                            flag=1;
                            //printf("send file1\n");
                            fileUpload(file->d_name ,buf.st_size, socket);
                            Recv(socket,buffer,200,0);

                            break;
                    }
                }
                 lkl=lkl->next;
		    }
		    if(flag==0)
		    {
		        //printf("send file 2\n");
		        fileUpload(file->d_name ,buf.st_size, socket);
                	Recv(socket,buffer,200,0);

		    }
		}
	}
	//Update fileInfo.dat
    storeInfoIntoFile();
	closedir(d);
	fclose(fp);
}
//send local file to server directly
int SendFile(const uchar *str, int socket)
{
	int i;
    uchar cpyStr[MAXLEN_CMD];
    uchar buffer[BUFSIZE];
    char *name;
	strcpy(cpyStr,str);
	strtok(cpyStr," \n");
	name=strtok(NULL," \n");
	i=fileUpload(name,0, socket);
	if(i==1)
	{
		Recv(socket,buffer,200,0);
	}
	return i;
}
//download a file from server
void fileDownload(uchar *fileName, int socket)
{
    FILE *fp;
    uchar buffer[BUFFER_SIZE];
    //ssize_t RecvBytes;
    int size;
	bzero(buffer,BUFFER_SIZE);
	printf("fileName=%s\n",fileName);
	if((fp=fopen(fileName,"wb"))==NULL)
	{
		printf("File Open Error!\n");
		exit(1);
	}
    //printf("Cleint. RECV\n");
    while(1)
	{
	    if(recv_file(buffer, &size, socket)==0)
	    {
	        printf("buffer=%s\tsize=%d\n",buffer,size);
	        system("clear");
	        printf("download successfully\n");
	        //printf("size=%d\n",size);
	        fwrite(buffer,size * sizeof(unsigned char),1,fp);
	        break;
	    }
	    printf("size=%d\n",size);
        fwrite(buffer,size * sizeof(unsigned char),1,fp);
        bzero(buffer,BUFFER_SIZE);

	}
	fclose(fp);
}
//deal with download command, and call fileDownload to download a file from server
void Download(const uchar *str, int socket)
{
    uchar cpyStr[MAXLEN_CMD];
    uchar fileName[20];
    uchar buffer[BUFSIZE];
    ssize_t RecvBytes;
	bzero(buffer,BUFSIZE);
    bzero(fileName,20);
    strcpy(cpyStr,str);
    strtok(cpyStr," \n");
    strcpy(fileName,strtok(NULL," \n"));
    //Send(socket,str,BUFSIZE,0);
    send_message(str,strlen(str),'1', socket);
    fileDownload(fileName, socket);
}
//upload a file, and be called in "Synchronize" and SendFile
int fileUpload(char *fileName,int fileSize, int socket)
{
    FILE *fp;
    uchar buffer[BUFFER_SIZE];
    int size;

	if((fp=fopen(fileName,"rb"))==NULL)
	{
		//printf("File Open Error: maybe no such file!\n");
		return -1;
	}
	sprintf(buffer, "send %s %d",fileName, fileSize);
	//Send(socket,buffer,BUFFER_SIZE,0); //tell Server that the client will send file to you
	send_message(buffer,strlen(buffer),'1', socket);
	bzero(buffer,BUFFER_SIZE);
    while((size = fread(buffer,1,BUFFER_SIZE,fp)) > 0)//fiel begin to be sent here
    {
        //Send(socket,buffer,BUFFER_SIZE,0);
        if(size<BUFFER_SIZE)
            send_message(buffer,size,'1', socket);
        else
            send_message(buffer,size,'0', socket);
        bzero(buffer,BUFFER_SIZE);
    }
	fclose(fp);

	printf("%s send Successfully\n",fileName);
	return 1;

}
//remove every records from .fileInfo.dat
void clsRecords()
{
    FILE *fp;
	if((fp=fopen(".fileInfo.dat","w"))==NULL)
	{
		printf("File Open Error!\n");
		exit(1);
	}
	fclose(fp);
}
// show statistics
void show(int frmSend, int frmRecv, int frmErr)
{
	printf("===============================\n");
	printf(" # of frame sent:\t%d\n",frmSend);
	printf(" # of frame recv:\t%d\n",frmRecv);
	printf(" # of frame erro:\t%d\n",frmErr);
	printf("===============================\n");
}
