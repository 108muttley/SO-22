#include "include/sdstore.h"

char* createBufArgs(int argc, char* argv[]){
    int tam = 0;
    int i;
    int k = 0;
    for(i = 1; i < argc ; i++) tam += strlen(argv[i]);
    char * buf = malloc(sizeof(char) * (tam + argc-2 + 1)); // tamanho dos argumentos + o numero de espaços + \0
    for(i = 1 ; i<argc; i++){
        strcpy(buf+k,argv[i]);
        k += strlen(argv[i]);
        if(i != argc-1){
            buf[k++] = ' ';
        }
    }
    buf[k] = '$';
    buf[k+1] = '\0';
    return buf;
}

int fd_cliente = 0, res = 0;


int main(int argc, char *argv[]){
	char buf[1], buffer[MAX_SIZE]="";
	int fd_cl_sv_write;
    int pid;
    char recebido[1024]="";
    int atual = 0;

    if((fd_cl_sv_write = open("fifo c->s",O_WRONLY)) == -1){ // open named pipe for write (cliente -> sv)
        perror("open");
        return -1;
    }
    else 
        printf("[DEBUG] opened fifo cl-sv for [writing]\n");

    /*if((fd_sv_cl_read = open("fifo s->c",O_RDONLY)) == -1){ // open named pipe for read (sv -> cliente)
       perror("open");
        return -1;
    }
    else
        printf("[DEBUG] opened fifo cl-sv for [reading]\n");*/

	pid = getpid();
	//printf("PID: %d\n", pid);

	snprintf(buffer, sizeof(buffer), "%s %d", "fifo", pid);

	if(mkfifo(buffer, 0666) == -1){
		perror("mkfifo cliente");
	}
	else printf("fifo aberto\n");


	if(argc>1){
    	if ((fork()) == 0){
    		if (strcmp(argv[1], "status") == 0){
    			char* aux = createBufArgs(argc, argv);
    			char input[MAX_SIZE]="";
    			snprintf(input, sizeof(input), "PID %d %s", pid, aux);
    			write(fd_cl_sv_write, input, strlen(input));
    			//printf("%s\n", aux);
    			char temp[] = "Status atual do server solicitado\n\0";
    			write(1, temp, strlen(temp));
    		}  
    		if(strcmp(argv[1], "exit") == 0){
		    	char* aux = createBufArgs(argc, argv);
		   		write(fd_cl_sv_write, aux, strlen(aux));
		   		close(fd_cl_sv_write);
                execlp("rm","rm",buffer,NULL);
    		}
    		
    		else if(strcmp(argv[1], "proc-file") == 0){
    			char* aux = createBufArgs(argc, argv);
                //printf("AUX: %s\n", aux);
    			char input[MAX_SIZE]="";
    			snprintf(input, sizeof(input), "PID %d %s", pid, aux);
    			write(fd_cl_sv_write, input, strlen(input));
    			char temp[] = "Proc-file Enviado\n";
    			write(1, temp, strlen(temp));
    		}
    		close(fd_cl_sv_write);
    		_exit(0);

    	}
    	else{
    		if((fd_cliente = open(buffer,O_RDONLY)) == -1){ // open named pipe for read (sv -> cliente)
       		perror("open");
        	return -1;
    		}
    		else
        		printf("[DEBUG] opened fifo cliente for [reading]\n");
    		while(read(fd_cliente, buf, 1) > 0){
                //printf("buf: %c\n", buf[0]);
                if (buf[0] == '\n'){
                    recebido[atual] = '\n';
                    recebido[atual+1] = '\0';
                    write(1, recebido, strlen(recebido));
                    recebido[0] = '\0';
                    atual = 0;
                }
                else if (buf[0] != '$'){
                    recebido[atual] = buf[0];
                    atual++;
                }
                else{
                    recebido[atual] = '\n';
                    recebido[atual+1] = '\0';
                    char* input = malloc(strlen(recebido) * sizeof(char));
                    strcpy(input, recebido);
                    write(1, recebido, strlen(recebido));
                    close(fd_cliente);
                    atual = 0;
                }
			}
    	}
    }
    else{
    	char temp[] = " ./sdstore status\n ./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n";
    	write(1, temp, strlen(temp));

    }

    close(fd_cl_sv_write);
    close(fd_cliente);
    execlp("rm","rm",buffer,NULL);

	return 0;
}
