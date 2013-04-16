all : Client Server
.PHONY : all

Client : Client.c wrapsock.c
	cc -o Client Client.c wrapsock.c

Server : Server.c wrapsock.c
	cc -o Server Server.c wrapsock.c





clean:
	rm Client Server  




