#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
ssize_t myreadln(int fd, char *line, size_t size);
char* createBuf(int argc, char* argv[]);

#endif