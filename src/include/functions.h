#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <sys/types.h>
#include <unistd.h>
ssize_t myreadln(int fd, char *line, size_t size);

#endif