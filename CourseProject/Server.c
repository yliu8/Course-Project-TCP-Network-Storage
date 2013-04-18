/**
 * Team:									Team 10 
 * Author:									Pe Zhang, Ya Liu, Hao Zhou  
 * Email									hzhou@wpi.edu
 * Date:									04/15/2013
 * Version:									1.0
 * Project ID:								Course Project
 * CS Class:								CS513
 * Programming Language:			        C
 * OS/Hardware dependencies:	            unix
 * Description:							    This is the source code for Server.
 * 											You can find more information about how to use it from ReadMe file.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Common.h"
#include "ServerFunc.h"
#include "wrapsock.h"
#include "protocol.h"

void HandleTCPClient(int );
static const int MAXPENDING = 10; // Maximum outstanding connection requests

int main(int argc, char *argv[])
{
	in_port_t servPort = atoi("5000");
	newsockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Construct local address structure
	struct sockaddr_in servAddr;
	bzero(&servAddr,  sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(servPort);
	Bind(newsockfd, (struct sockaddr*) &servAddr, sizeof(servAddr));
	Listen(newsockfd, MAXPENDING);// Mark the socket so it will listen for incoming connections

	for (;;)
	{
	    pid_t pid;
		struct sockaddr_in clntAddr; // Client address
		socklen_t clntAddrLen = sizeof(clntAddr);
		sockfd = Accept(newsockfd, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if((pid=fork())==0) //child process services the client, the parent process waits for another commection
        {
            close(newsockfd);
            HandleTCPClient(sockfd);
        }

	}
	close(sockfd);
}

void HandleTCPClient(int clntSocket)
{
	uchar buffer[MAX_FRM_PAYLOAD];
	uchar *pbuffer=buffer;
    int cmdCode=0;
    begin:
	bzero(buffer,MAX_FRM_PAYLOAD);
	//ssize_t numBytesRcvd = Recv(clntSocket, buffer, MAX_FRM_PAYLOAD, 0);
    recv_message(buffer, clntSocket);
	while(1)
	{
	    cmdCode=serverCmdCheck(buffer);
	    printf("cmd=%s\n",buffer);
	    switch(cmdCode)//deal with different commands below
	    {
            case 1:	switch(Register(buffer))
                    {
                        case 1: bzero(buffer,MAX_FRM_PAYLOAD); pbuffer="Register Successfully! You can login in now:\n";send_message(pbuffer,strlen(pbuffer),'1', clntSocket);goto begin;
						case 0: bzero(buffer,MAX_FRM_PAYLOAD); pbuffer="This name has already be used!\n"; send_message(pbuffer,strlen(pbuffer),'1', clntSocket); goto begin;
						default:bzero(buffer,MAX_FRM_PAYLOAD); pbuffer="Error to create folder for you!\n";send_message(pbuffer,strlen(pbuffer),'1', clntSocket);goto begin;
                    }
                    break;
            case 2: switch(Login(buffer))
                    {
                        case 1: bzero(buffer,MAX_FRM_PAYLOAD);pbuffer="Congratulations! You have logged on successfully\n";break;
                        case 0: bzero(buffer,MAX_FRM_PAYLOAD);pbuffer="0Wrong PassWord\n";send_message(pbuffer,strlen(pbuffer),'1', clntSocket);goto begin;break;
                        default: bzero(buffer,MAX_FRM_PAYLOAD);pbuffer="0No such user\n";send_message(pbuffer,strlen(pbuffer),'1', clntSocket);goto begin;break;
                    }
                    break;
            case 3: if(ModifyPwd(buffer)==1)
                    {
                        bzero(buffer,MAX_FRM_PAYLOAD);
                        pbuffer="Modify successfully!\n";
                    }
                    else
                    {
                        bzero(buffer,MAX_FRM_PAYLOAD);
                        pbuffer="Your old passWord is wrong!\n";
                    }
                    break;
            case 4: fileSend(buffer,clntSocket);goto test; break;
            case 5: if(Delete(buffer)==0)
                    {
                        bzero(buffer,MAX_FRM_PAYLOAD);
                        pbuffer="Delete Successfully!\n";
                    }
                    else
                    {
                        bzero(buffer,MAX_FRM_PAYLOAD);
                        pbuffer="Delete failed!\n";
                    }
                    break;
            case 6: ;continue;
            case 7: List(clntSocket);
                    bzero(buffer,MAX_FRM_PAYLOAD);
                    pbuffer="------------------\n";
                    break;
            case 8: bzero(buffer,MAX_FRM_PAYLOAD);
                    pbuffer="quit";
                    break;
            case 9: fileRecv(buffer,clntSocket);
                    bzero(buffer,MAX_FRM_PAYLOAD);
                    //pbuffer="Send successfully!\n";
			system("clear");
                    goto fileRecvlbl;
                    break;
            default: goto test;break;
	    }
	    printf("to Send=%s",pbuffer);
        //Send(clntSocket, pbuffer, MAX_FRM_PAYLOAD, 0);
        send_message(pbuffer,strlen(pbuffer),'1', clntSocket);
        if(strncasecmp("Quit",buffer,4)==0)
            Quit(clntSocket);
        bzero(buffer,MAX_FRM_PAYLOAD);
        fileRecvlbl:
        pbuffer="End"; // tell the client that the server have finish its last command request
        printf("buffer=%s\n",pbuffer);
        //Send(clntSocket, pbuffer, MAX_FRM_PAYLOAD, 0);
        send_message(pbuffer,strlen(pbuffer),'1', clntSocket);
        test:
        bzero(buffer,MAX_FRM_PAYLOAD);
        //ssize_t numBytesRcvd = Recv(clntSocket, buffer, MAX_FRM_PAYLOAD, 0);
        recv_message(buffer, clntSocket);
	}
	//close(clntSocket);
}

