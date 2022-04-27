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
