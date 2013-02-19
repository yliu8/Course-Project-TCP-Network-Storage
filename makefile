all : sender receiver
.PHONY : all 

sender : sender.o   
	cc -o sender sender.o   

receiver : receiver.o   
	cc -o receiver receiver.o   
	

sender.o: sender.c 
	cc -c sender.c 

receiver.o: receiver.c 
	cc -c receiver.c 


	
clean:
	rm sender receiver  sender.o receiver.o 




