#include "include/functions.h"
#include <stdio.h>
ssize_t myreadln(int fd, char *line, size_t size){
	int bytes_read = 0;
	int size1 = 0;
	char buf;

	while((bytes_read = read(fd, &buf, 1)) == 1 && size1<size){
		if(buf == '\n'){
			*(line+size1) = '\0';
			return size1;
		}
		*(line+size1) = buf;
		size1++;
	}
	return size1;
}

char* createBuf(int argc, char* argv[]){
    int tam = 0;
    int i;
    int k = 0;
    for(i = 0; i < argc ; i++) tam += strlen(argv[i]);
    char * buf = malloc(sizeof(char) * (tam + argc-2 + 1)); // tamanho dos argumentos + o numero de espaços + \0
    for(i = 1 ; i<argc; i++){
        strcpy(buf+k,argv[i]);
        k += strlen(argv[i]);
        if(i != argc-1){
            buf[k++] = ' ';
        }
    }
    buf[k] = '\0';
    return buf;
}
