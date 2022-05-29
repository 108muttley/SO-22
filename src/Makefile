CC = gcc -Wall

all: mkfifo sdstored sdstore

mkfifo: mkfifo.c 
	$(CC) mkfifo.c -o mkfifo

sdstore: sdstore.c
	$(CC) sdstore.c functions.c -o sdstore

sdstored: sdstored.c
	$(CC) sdstored.c functions.c -o sdstored

clean:
	@rm -rf *.dSYM
	@rm fifo*
	@rm mkfifo
	@rm sdstored
	@rm sdstore




