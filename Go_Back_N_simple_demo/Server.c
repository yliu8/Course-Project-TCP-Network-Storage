#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>                //for time_t and time
#include "protocol.h"
#include "wrapsock.h"

#define SERVER_PORT 7754
#define LENGTH_OF_LISTEN_QUEUE 10



int main(int argc, char **argv)
{
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	packet pkt;

	newsockfd = Socket(AF_INET,SOCK_STREAM,0);
	Bind(newsockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
	Listen(newsockfd, LENGTH_OF_LISTEN_QUEUE) ;

	while (true)
	{

		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);
		sockfd = Accept(newsockfd,(struct sockaddr*)&client_addr,&length); //Accept need "continue" not "exit"

		if((stream = fopen("server.jpg","rb"))==NULL)
		{
			printf("The file was not opened! \n");
			exit(1);
		}
		Data_Link_Layer_Send();
		printf("Server\n");
		fclose(stream);
		close(sockfd);
	}

	close(newsockfd);
	return 0;
}
