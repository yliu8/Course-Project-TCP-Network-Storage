#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>                //for time_t and time
#include <arpa/inet.h>
#include "protocol.h"
#include "wrapsock.h"

#define SERVER_PORT    7754

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s ServerIPAddress\n",argv[0]);
		exit(1);
	}

	//time_t now;
    packet pkt;

	struct sockaddr_in client_addr;
	bzero(&client_addr,sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY means automatically get the local IP
	client_addr.sin_port = htons(0);    //0 means get a free port assgned by System

	sockfd = Socket(AF_INET,SOCK_STREAM,0);
	Bind(sockfd,(struct sockaddr*)&client_addr,sizeof(client_addr));

	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if(inet_aton(argv[1],&server_addr.sin_addr) == 0)
	{
		printf("Server IP Address Error!\n");
		exit(1);
	}
	server_addr.sin_port = htons(SERVER_PORT);
	socklen_t server_addr_length = sizeof(server_addr);

	Connect(sockfd,(struct sockaddr*)&server_addr, server_addr_length) ;


	/**********************Connection Established************************/
	uchar buffer[MAX_BYTE_STREAM_SIZE];

	if((stream = fopen("fromServer.jpg","wb"))==NULL)
	{
		printf("The file was not opened! \n");
		exit(1);
	}
	Data_Link_Layer_Recv();

	printf("Recieve File From Server[%s] Finished\n", argv[1]);

	fclose(stream);

	close(sockfd);
	return 0;
}
