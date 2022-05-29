CC = gcc -Wall

all: server client

server: sdstored

client: sdstore

sdstore: sdstore.c
	$(CC) sdstore.c -o sdstore

sdstored: sdstored.c
	$(CC) sdstored.c -o sdstored

clean:
	@rm -rf *.dSYM
	@rm fifo*
	@rm mkfifo
	@rm sdstored
	@rm sdstore

