all: client server

dclient:
	gcc -pthread -o client client.c
	
dserver:
	gcc -pthread -o server server.c

clean: 
	rm -f client server
