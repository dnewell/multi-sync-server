all: client server

client:
	gcc -g -pthread -o client client.c
	
server:
	gcc -g -pthread -o server server.c

clean: 
	rm -f client server
