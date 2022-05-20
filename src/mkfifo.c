#include "include/sdstore.h"


int main(int argc, char const *argv[]){

	if(mkfifo("fifo c->s", 0666) == -1){
		perror("mkfifo c->s");
	}
	else printf("fifo cs aberto\n");

	if(mkfifo("fifo s->c", 0666) == -1){
		perror("mkfifo s->s");
	}
	else printf("fifo aberto\n");

	return 0;
}
