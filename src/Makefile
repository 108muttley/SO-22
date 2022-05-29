all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o
		gcc -g obj/sdstored.o -o bin/sdstored

obj/sdstored.o: sdstored.c
		gcc -Wall -g -c sdstored.c -o obj/sdstored.o

bin/sdstore: obj/sdstore.o
		gcc -g obj/sdstore.o -o bin/sdstore

obj/sdstore.o: sdstore.c
		gcc -Wall -g -c sdstore.c -o obj/sdstore.o

clean:
	rm obj/* bin/sdstored bin/sdstore