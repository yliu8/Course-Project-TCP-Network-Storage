/**
 * Author:									Zhou Hao
 * Email										hzhou@wpi.edu
 * Date:										02/04/2013
 * Version:									1.0
 * Project ID:								Program 1
 * CS Class:								CS513
 * Programming Language:			C
 * OS/Hardware dependencies:	unix
 * Description:							This is the source code for TCP Client.
 * 												I finish Program 1 on the base of Program 0.
 * 												You can find more information about how to use it from ReadMe file.
 **/

#include <stdio.h>
#include <stdlib.h>		// for atoi()
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>//for socket(), connect(), send(), and recv()
#include <netinet/in.h>
#include <arpa/inet.h>	//for sockaddr_in and inet_addr()
#include "Common.h"
#include "ClientFunc.h"
#include "wrapsock.h"
#include "protocol.h"

int main(int argc, char *argv[])
{
    int cmdCode=0;
    int IsConnected=0;
    int c;
	if (argc != 2)
	{
	    printf("Parameters: ./Client <Server IP> ");
		exit(1);
	}

	//===========Deal with Before Logged (Only Register, Login, Help work here)============
    welcome();
    begin:
	fgets(inputStr,MAX_FRM_PAYLOAD,stdin);
	cmdCode=CmdFormatCheck(inputStr);
	while(cmdCode==-1)
	{
		printf("Too long command. Please re-input again:\n");
		if(strlen(inputStr)>=99)
				while((c = getchar()) != '\n' && c != EOF);
		fflush(stdin);
		fgets(inputStr,MAX_FRM_PAYLOAD,stdin);
		cmdCode=CmdFormatCheck(inputStr);
	}
	while(beforeLogged(inputStr)==0)
	{
		fgets(inputStr,MAX_FRM_PAYLOAD,stdin);
		if(strlen(inputStr)>=99)
				while((c = getchar()) != '\n' && c != EOF);
		fflush(stdin);
	}
	//=====End of Dealing with Before Logged (inputStr contains infomation for Register command or Login command)=========

	//===========initiate the socket =====================
	if(IsConnected==0)
	{
        char *servIP=argv[1];
        in_port_t servPort =atoi("5000");// Set server port (numeric) 5000
        sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// Create a reliable, stream socket using TCP
        struct sockaddr_in servAddr;// Construct the server address structure
        bzero(&servAddr,sizeof(servAddr)); // Zero out structure
        servAddr.sin_family = AF_INET;			   // IPv4 address family
        int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);	// Convert address
        if (rtnVal <= 0)
        {
            printf("inet_pton() failed\n");
            exit(1);
        }
        servAddr.sin_port = htons(servPort);		// Server port
	//===========END initiate the socket =====================


	//===========Establish the connection to the echo server=============

	    Connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr));
	    IsConnected=1;
	}

	uchar buffer[MAX_FRM_PAYLOAD];
	ssize_t RecvBytes;
	int IsPrint=0;
	for(;;)
	{
	    IsPrint=0;
		//Send(sockfd, inputStr, MAX_FRM_PAYLOAD, 0);
		send_message(inputStr,strlen(inputStr),'1', sockfd);
        bzero(buffer,MAX_FRM_PAYLOAD);
        while(1)
        {

            //RecvBytes = Recv(sockfd,buffer,MAX_FRM_PAYLOAD,0);
            recv_message(buffer, sockfd);
            if(strncasecmp("End",buffer,3)==0 )
            {
                break;
            }
            if(cmdCode==1)
            {
                printf("%s\n",buffer);
                goto begin;
            }
            if(cmdCode==2 && buffer[0]=='0')
            {
                printf("%s\n",buffer+1);
                goto begin;
            }
            else
            {
                if(strncasecmp("Quit",buffer,4)==0)
                {
                    printf("Good Bye!\n");
                    Quit(sockfd);
                }
                else
                    printf("%s\n",buffer);
            }

            bzero(buffer,MAX_FRM_PAYLOAD);
        }
		for(;;)
		{

			int breakControl=1;
            bzero(inputStr,MAX_FRM_PAYLOAD);
			fgets(inputStr,MAX_FRM_PAYLOAD,stdin);
            cmdCode=CmdFormatCheck(inputStr);
            while(cmdCode==-1)
			{
				printf("Too long command. Please re-input again:\n");
				if(strlen(inputStr)>=99)
						while((c = getchar()) != '\n' && c != EOF);
				fflush(stdin);
				fgets(inputStr,MAX_FRM_PAYLOAD,stdin);
				cmdCode=CmdFormatCheck(inputStr);
			}
            switch(cmdCode)
            {
                case  1: printf("No registration here\n");breakControl=0;break;
                case  2: printf("You have already logged\n");breakControl=0;break;
                case  3: ;break;//ModifyPwd oldPassword newPassword#
                case  4: Download(inputStr,sockfd);breakControl=0;break;
                case  5: ;break;//Delete
                case  6: Synchronize(sockfd);printf("---------------------\n");breakControl=0;break;
                case  7: ;break; //List
                case  8: helpInfo();breakControl=0;break;
                case  9: ;break;//quit(quit after get info from Server)
                case 10: if(SendFile(inputStr,sockfd)==-1) printf("No such File\n");breakControl=0;break;
                case 11: clear();breakControl=0;break;
                case 12: clsRecords();printf("Your records are cleaned. \nNow you can synchronize from scratch!\n");breakControl=0;break;
                case 13: ;break;
                case 14: show(frmSend,frmRecv,frmErr);breakControl=0;break;
                default: printf("Command not support here, please re-input below:\n");breakControl=0;break;
            }
            if(breakControl==1)
            {
                break;
            }
		}

	}

}
