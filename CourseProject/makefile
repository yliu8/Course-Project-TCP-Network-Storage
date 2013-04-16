all : Client Server
.PHONY : all

Client : Client.o   ClientFunc.o Common.o wrapsock.o
	cc -o Client Client.o   ClientFunc.o Common.o wrapsock.o  

Server : Server.o   ServerFunc.o Common.o wrapsock.o
	cc -o Server Server.o   ServerFunc.o Common.o wrapsock.o  


Client.o: Client.c  Common.c ClientFunc.c  wrapsock.c
	cc -c Client.c  Common.c ClientFunc.c  wrapsock.c

Server.o: Server.c  Common.c ServerFunc.c  wrapsock.c
	cc -c Server.c  Common.c ServerFunc.c  wrapsock.c



clean:
	rm Client Server  *.o




