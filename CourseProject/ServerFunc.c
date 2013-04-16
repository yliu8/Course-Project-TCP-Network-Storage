#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "Common.h"
#define BUFSIZE 100
#define BUFFER_SIZE 190
typedef struct user
{
	uchar userName[10];
	uchar userPassword[10];
	struct user *next;
}Node, *LinkedList;

//if the name already exists, return 1, otherwise return 0
int nameDupCheck(char *nameStr)
{
    DIR *d;
	struct dirent *file;
	struct stat buf;
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

		if(stat(file->d_name, &buf) >= 0 && S_ISDIR(buf.st_mode) )
		{
			if(strcmp(file->d_name,nameStr)==0)
                return 1;
		}
	}
	closedir(d);
    return 0;
}

//Create new record in the text file and create a folder for the user. Make sure no name duplicated
int Register(uchar *str)
{
    uchar cpyStr[MAXLEN_CMD];
    char *name;
    char *passWord;
    FILE *fp;
	strcpy(cpyStr,str);
	strtok(cpyStr," ");
	name=strtok(NULL," ");
    passWord=strtok(NULL," ");
    //printf("name=%s\n",name);
    if(nameDupCheck(name)==0)//first: check namewhether it is duplicated
    {
        if((mkdir(name,00777))==-1)
        {
            fputs("error in creating dir\n", stdout);
            printf("%s\n",strerror(errno));
            return -1;
        }
        chdir(name);
        if((fp=fopen(".passWord.dat","wb"))==NULL)
        {
            printf("File Open Error!\n");
            exit(1);
        }
        fwrite(passWord, 10*sizeof(unsigned char),1,fp);
        fclose(fp);
        chdir("../");
        return 1;
    }
    else
    {
        return 0;
    }
}
//success returns 1, wrong passWord returns 0, no such user returns -1
int Login(uchar *str)
{
    uchar cpyStr[MAXLEN_CMD];
    char *name;
    char *passWord;
    char buffer[20];
    FILE *fp;
	strcpy(cpyStr,str);
	strtok(cpyStr," ");
	name=strtok(NULL," ");
    passWord=strtok(NULL," ");
    if(nameDupCheck(name)==1) //Duplicated means name exists, then read the passWord for this usrName
    {
        chdir(name);//this is very important
        if((fp=fopen(".passWord.dat","rb"))==NULL)
        {
            printf("File Open Error!\n");
            exit(1);
        }

        fread(buffer,1,10,fp);
        fclose(fp);
        if(strncasecmp(passWord,buffer,strlen(buffer))==0)
        {
            return 1;//success
        }
        else
        {
            chdir("../");
            return 0;//wrong passWord
        }

    }
    else
    {
        return -1; //no such user
    }
}
//modify password: success return 1; fail return 0.
int ModifyPwd(uchar *str)
{
    uchar cpyStr[MAXLEN_CMD];
    char *oldPwd;
    char *newPwd;
    char buffer[20];
    FILE *fp;
	strcpy(cpyStr,str);
	strtok(cpyStr," ");
	oldPwd=strtok(NULL," ");
    newPwd=strtok(NULL," ");
    if((fp=fopen(".passWord.dat","rb"))==NULL)
    {
        printf("File Open Error!\n");
        exit(1);
    }
    fread(buffer,1,11,fp);
    fclose(fp);
    if(strncasecmp(oldPwd,buffer,strlen(buffer)-1)==0)//-1 remove'\n'
    {
        if((fp=fopen(".passWord.dat","wb"))==NULL)
        {
            printf("File Open Error!\n");
            exit(1);
        }
        fwrite(newPwd, 10*sizeof(unsigned char),1,fp);
        fclose(fp);
        return 1;//success
    }
    else
    {
        return 0;//failed
    }

}
//receive file from client
void fileRecv(uchar *str, int socket)
{
    FILE *fp;
    uchar buffer[BUFFER_SIZE];
    ssize_t RecvBytes;
    uchar size;
    uchar cpyStr[MAXLEN_CMD];
    uchar fileDir[20];
    bzero(fileDir,20);
	strcpy(cpyStr,str);
	strtok(cpyStr," ");
    strcpy(fileDir,strtok(NULL," "));
	bzero(buffer,BUFFER_SIZE);
	if((fp=fopen(fileDir,"wb"))==NULL)
	{
		printf("File Open Error!\n");
		exit(1);
	}
	printf("RECV\n");
	while(1)
	{
	    if(recv_file(buffer, &size, socket)==0)
	    {
	        printf("buffer=%s\tsize=%d\n",buffer,size);
	        fwrite(buffer,size * sizeof(unsigned char),1,fp);
	        break;
	    }
	    printf("buffer=%s\tsize=%d\n",buffer,size);
        fwrite(buffer,size * sizeof(unsigned char),1,fp);
        bzero(buffer,BUFFER_SIZE);

	}

	fclose(fp);
}
//send file to client
void fileSend(uchar *str, int socket)
{
    FILE *fp;
    uchar buffer[BUFFER_SIZE];
    int size;
    uchar cpyStr[MAXLEN_CMD];
    uchar fileDir[20];
    bzero(fileDir,20);
	strcpy(cpyStr,str);
	strtok(cpyStr," \n");
    strcpy(fileDir,strtok(NULL," \n"));

	bzero(buffer,BUFFER_SIZE);
	printf("file=%s\n",fileDir);
	if((fp=fopen(fileDir,"rb"))==NULL)
	{
		printf("File Open Error!\n");
		exit(1);
	}

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

}
//scan the file in the user folder, and return their infomation of file name, file size to the client side
void List(int socket)
{
    DIR *d;
	struct dirent *file;
	struct stat buf;
	int index=1;
    uchar buffer[BUFSIZE];
    bzero(buffer,BUFSIZE);

	if(!(d = opendir(".")))
	{
		printf("error opendir!!!\n");
		exit(1);
	}
	while((file = readdir(d)) != NULL)
	{
		if(strncmp(file->d_name, ".", 1) == 0)
            continue;

		if(stat(file->d_name, &buf) >= 0 && !S_ISDIR(buf.st_mode) )
		{
		    sprintf(buffer,"%3d. %-20s %ld", index++,file->d_name, buf.st_size);
			//Send(socket,buffer,BUFSIZE,0);
            send_message(buffer,strlen(buffer),'1', socket);
            bzero(buffer,BUFSIZE);
		}
	}
	closedir(d);

}
//0 means removed
int Delete(uchar *str)
{
    uchar cpyStr[MAXLEN_CMD];
    strcpy(cpyStr,str);
    strtok(cpyStr," \n");
    return remove(strtok(NULL," \n"));
}
//close the socket and exit
void Quit(int socket)
{
    close(socket);
    printf("disconnected\n");
    exit(0);
}
