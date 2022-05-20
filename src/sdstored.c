#include "include/sdstore.h"


typedef struct struct_configuration{
	int nop;
	int bcompress;
	int bdecompress; 
	int gcompress;
	int gdecompress;
	int encrypt;
	int decrypt;
	int maxNop;
	int maxBcompress;
	int maxBdecompress;
	int maxGcompress;
	int maxGdecompress;
	int maxEncrypt;
	int maxDecrypt;
	char* path_exec;
}*Configuration;

typedef struct tarefa{
	int idCliente;
	char* inputfile;
	char* outputfile;
	int pidFilho;
	char** transf; //
	char* string;
	int nrTransf; //número de transformações
	int status; //1 - em execução, 2 - pending, 3 - terminada
	int priority; //0 a 5 (sendo 5 máxima prioridade)
}*Tarefa;

Tarefa* tarefas;
Tarefa* stack;
int current = -1, maxTarefas = 0, currentQ = -1, maxQueue = 0, maxClientes = -1;
Configuration configuration;
int fd_cl_sv_read, fd_sv_cl_write;
int fd_clientes[50];


void signIntHandler(int signum){
	close(fd_cl_sv_read);
	close(fd_sv_cl_write);
	if(fork() == 0){
		execlp("rm","rm","fifo c->s",NULL);
		_exit(0);
	}
	if(fork() == 0){
		execlp("rm","rm","fifo s->c",NULL);
		_exit(0);
	}

	for(int i = 0; i<3;i++){
		wait(0L);
	}
	for(int i = 0; i<maxTarefas;i++){
		if(tarefas[i]->inputfile) free(tarefas[i]->inputfile);
		if (tarefas[i]->outputfile) free(tarefas[i]->outputfile);
		if(tarefas[i]->string) free(tarefas[i]->string);
		for(int j = 0; j<tarefas[i]->nrTransf; j++){
			if(tarefas[i]->transf[j]) free(tarefas[i]->transf[j]);
		}
		free(tarefas[i]->transf);
		free(tarefas[i]);
	}
	for(int i = 0; i<currentQ;i++){
		if(stack[i]->inputfile) free(stack[i]->inputfile);
		if (stack[i]->outputfile) free(stack[i]->outputfile);
		if(stack[i]->string) free(stack[i]->string);
		for(int j = 0; j<stack[i]->nrTransf; j++){
			if(stack[i]->transf[j]) free(stack[i]->transf[j]);
		}
		free(tarefas[i]->transf);
		free(tarefas[i]);
	}

	if(configuration->path_exec) free(configuration->path_exec);

	free(tarefas);
	free(stack);
	_exit(0);
}


//lê o ficheiro conf para guardar as informações do máximo de cada transformação e guardar o path para os executáveis
void init_Configuration(char* filename, char* path){

	int read_bytes = 0;
	int i= 0;
	int conf[7];
	char buf[MAX_SIZE];
	char* token;
	configuration = malloc(sizeof(struct struct_configuration));
	tarefas = malloc(MAX_TAREFA * sizeof(Tarefa));
	stack = malloc(MAX_TAREFA * sizeof(Tarefa));

	int fileConf_fd = open(filename, O_RDONLY);
	if(fileConf_fd < 0){
		perror("open Configuration File");
		return;
	}

	while((read_bytes = myreadln(fileConf_fd, buf, MAX_SIZE)) > 0){

		token = strtok(buf, " ");
		printf("%s\n",token);
		token = strtok(NULL, " ");
		conf[i] = atoi(token);
		printf("[DEBUG] token:%d\n", conf[i]);
		i++;
	}
	configuration->nop = conf[0];
	configuration->maxNop = conf[0];
	configuration->bcompress = conf[1];
	configuration->maxBcompress = conf[1];
	configuration->bdecompress = conf[2];
	configuration->maxBdecompress = conf[2];
	configuration->gcompress = conf[3];
	configuration->maxGcompress = conf[3];
	configuration->gdecompress = conf[4];
	configuration->maxGdecompress = conf[4];
	configuration->encrypt = conf[5];
	configuration->maxEncrypt = conf[5];
	configuration->decrypt = conf[6];
	configuration->maxDecrypt = conf[6];
	configuration->path_exec = malloc(strlen(path)*sizeof(char));
	strcpy(configuration->path_exec,path);

	close(fileConf_fd);
}

int exec_command(char* command)
{
	int exec_ret;
	char buffer[100] = "";
	snprintf( buffer, sizeof(buffer), "%s%s%s", configuration->path_exec, "/", command);
	exec_ret = execvp(buffer, NULL);

	return exec_ret;
}

int temRecursos(char** comandos, int nrComandos, int* nop, int* bcompress, int* bdecompress, int* gcompress, int* gdecompress, int* encrypt, int* decrypt){
	int  i;
	for(i=0; i<nrComandos;i++){
		if (!strcmp(comandos[i], "nop")) (*nop)++;
			else if (!strcmp(comandos[i], "bcompress")) (*bcompress)++;
			else if (!strcmp(comandos[i], "bdecompress")) (*bdecompress)++;
			else if (!strcmp(comandos[i], "gcompress")) (*gcompress)++;
			else if (!strcmp(comandos[i], "gdecompress")) (*gdecompress)++;
			else if (!strcmp(comandos[i], "encrypt")) (*encrypt)++;
			else if (!strcmp(comandos[i], "decrypt")) (*decrypt)++;
			else {
				printf("[DEBUG] Tarefa Inválida\n");
				return -1;
			}
	}
	if (!(((configuration->nop - *nop) < 0) || 
			((configuration->bcompress - *bcompress) < 0) || 
			((configuration->bdecompress - *bdecompress) < 0) ||
			((configuration->gcompress - *gcompress) < 0) ||
			((configuration->gdecompress - *gdecompress) < 0) ||
			((configuration->encrypt - *encrypt) < 0) ||
			((configuration->decrypt - *decrypt) < 0))) 
		return 1;
	else return 0;
}

int exec_tarefa(char** comandos, int nrComandos, char* inputfile, char* outputfile){
	int filein = open(inputfile, O_RDONLY);
	int pipes[nrComandos-1][2];
	if(filein < 0){
		perror("open input file");
		return -1;
	}

	int fileout = open(outputfile, O_CREAT | O_TRUNC | O_WRONLY, 0640);
	if(fileout < 0){
		perror("open output file");
		return 	-1;
	}
	for (int c = 0; c < nrComandos; ++c){
		if (c==0){
			if (pipe(pipes[c]) != 0)
			{
				perror("pipe");
				return -1;
			}					
			switch(fork()){
				case -1: 
					perror("fork");
					return -1;
				case 0:
					close(pipes[c][0]);
					dup2(filein,0); 
					close(filein);
					dup2(pipes[c][1],1);
					close(pipes[c][1]);
					exec_command(comandos[c]);
					_exit(0);
				default :
					close(pipes[c][1]);

			}
		}
		else if (c == nrComandos-1){
			if (pipe(pipes[c]) != 0)
			{
				perror("pipe");
				return -1;
			}
			switch(fork()){
				case -1:
					perror("fork");
					return -1;
				case 0:
					close(pipes[c][1]);
					dup2(fileout,1);
					close(fileout);
					dup2(pipes[c-1][0],0);
					close(pipes[c-1][0]);
					exec_command(comandos[c]);
					_exit(0);
				default:
					close(pipes[c-1][0]);
			}
		}
		else{			
			if (pipe(pipes[c]) != 0)
			{
				perror("pipe");
				return -1;
			}
			switch(fork()){
				case -1:
					perror("fork");
					return -1;

				case 0:
					dup2(pipes[c-1][0],0);
					close(pipes[c-1][0]);
					dup2(pipes[c][1],1);
					close(pipes[c][1]);
					exec_command(comandos[c]);
					_exit(0);
				default:
					close(pipes[c-1][0]);
					close(pipes[c][1]);

			}
		}
		
	} 
	int j = 0;
	while(j<nrComandos){
		wait(NULL);
		j++;
	}
	int fd_temp;
	if((fd_temp = open("fifo c->s", O_WRONLY)) == -1){
		perror("fifo cl-sv pelo filho");
	}
	else{
		char buffer[MAX_SIZE] = "";
 		sprintf(buffer, "%s %s %d%s", "FINISH", "task:", current+1, "\0");
 		sleep(20);
		printf("Vai imprimir\n");
		printf("CURRENT: %d\n",tarefas[current]->idCliente);
		write(fd_temp, buffer, strlen(buffer));
		printf("ola\n");
		write(fd_clientes[tarefas[current]->idCliente], "Concluded\n", 10);
	}
	close(fd_temp);
	close(fd_clientes[tarefas[current]->idCliente]);
	_exit(0);

}


int interpreter(char* input){
	printf("INPUT: %s\n", input);
	char* aux = malloc(strlen(input) * sizeof(char));
	strcpy(aux, input);
	char* string = strtok(input, " ");
	char* inputfile, *outfile;
	char** comandos;
	comandos = malloc(30 * sizeof(char*));
	int pid=0, nrComandos=0,i=0, nop=0, bcompress=0, bdecompress=0, gcompress=0, gdecompress=0, encrypt=0, decrypt=0;
	if (strcmp(string, "proc-file") == 0){
		string = strtok(NULL, " ");
		inputfile = malloc(strlen(string) * sizeof(char));
		strcpy(inputfile, string);
		string = strtok(NULL, " ");
		outfile = malloc(strlen(string) * sizeof(char));
		strcpy(outfile, string);
		string = strtok(NULL, " ");
		for (i=0; string != NULL; i++) {
			comandos[i] = malloc((strlen(string) * sizeof(char)));
			strcpy(comandos[i], string);
			string = strtok(NULL, " \0");
		}

		nrComandos = i;
		maxTarefas++;
		if (temRecursos(comandos, nrComandos, &nop, &bcompress, &bdecompress, &gcompress, &gdecompress, &encrypt, &decrypt)){
			current++;
			tarefas[current] = malloc(1 * sizeof(struct tarefa));
			tarefas[current]->transf = comandos;
			tarefas[current]->nrTransf = nrComandos;
			tarefas[current]->string = aux;
			tarefas[current]->pidFilho = pid;
			tarefas[current]->priority = 0;
			tarefas[current]->idCliente = maxClientes;
			tarefas[current]->status = 1; //em execução 
			tarefas[current]->inputfile = inputfile;
			tarefas[current]->outputfile = outfile;
			configuration->nop = configuration->nop - nop;
			configuration->bcompress = configuration->bcompress - bcompress;
			configuration->bdecompress = configuration->bdecompress - bdecompress;
			configuration->gcompress = configuration->gcompress - gcompress;
			configuration->gdecompress = configuration->gdecompress - gdecompress;
			configuration->encrypt = configuration->encrypt - encrypt;
			configuration->decrypt = configuration->decrypt - decrypt;
			write(fd_clientes[tarefas[current]->idCliente], "Processing\n", 11);
			printf("NOP: %d\n", nop);
			printf("Config : NOP -> %d\n", configuration->nop );
			if((pid = fork()) == 0){
				exec_tarefa(comandos, nrComandos, inputfile, outfile);
			}

		}
		else{
			maxQueue++;
			currentQ++;
			stack[currentQ] = malloc(1 * sizeof(struct tarefa));
			stack[currentQ]->transf = comandos;
			stack[currentQ]->nrTransf = nrComandos;
			stack[currentQ]->idCliente = maxClientes;
			stack[currentQ]->string = aux;
			stack[currentQ]->pidFilho = 0;
			stack[currentQ]->priority = 0;
			stack[currentQ]->outputfile = outfile;
			stack[currentQ]->inputfile = inputfile;
			stack[currentQ]->status = 2;
			write(fd_clientes[maxClientes], "Pending\n", 8);
			printf("Acrescentado à stack\n");

		}
	}
	else if(strcmp("status", string) == 0){
		if (fork()==0){
			for (int i = 0; i < maxTarefas; ++i){
				printf("Entrei\n");
				if (tarefas[i]->status == 1){
					char* token = malloc(strlen(tarefas[i]->string) * sizeof(char));
					strcpy(token, tarefas[i]->string);
					char buffer[MAX_SIZE] = "";
					sprintf(buffer, "%s%d%s %s\n", "task #", i+1, ":", token );
					write(fd_clientes[maxClientes], buffer, strlen(buffer));
					free(token);
				}
			}
			char nopAux[MAX_SIZE]="", bcompressAux[MAX_SIZE]="", bdecompressAux[MAX_SIZE]="", gcompressAux[MAX_SIZE]="", gdecompressAux[MAX_SIZE]="", encryptAux[MAX_SIZE]="", decryptAux[MAX_SIZE]="";
			snprintf(nopAux, sizeof(nopAux), "transf nop: %d/%d (running/max)\n", (configuration->maxNop-configuration->nop), configuration->maxNop);
			snprintf(bcompressAux, sizeof(bcompressAux), "transf bcompress: %d/%d (running/max)\n", (configuration->maxBcompress - configuration->bcompress), configuration->maxBcompress);
			snprintf(bdecompressAux, sizeof(bdecompressAux), "transf bdecompress: %d/%d (running/max)\n", (configuration->maxBdecompress-configuration->bdecompress), configuration->maxBdecompress);
			snprintf(gcompressAux, sizeof(gcompressAux), "transf gcompress: %d/%d (running/max)\n", (configuration->maxGcompress - configuration->gcompress), configuration->maxGcompress);
			snprintf(gdecompressAux, sizeof(gdecompressAux), "transf gdecompress: %d/%d (running/max)\n", (configuration->maxGdecompress-configuration->gdecompress), configuration->maxGdecompress);
			snprintf(encryptAux, sizeof(encryptAux), "transf encrypt: %d/%d (running/max)\n", (configuration->maxEncrypt-configuration->encrypt), configuration->maxEncrypt);
			snprintf(decryptAux, sizeof(decryptAux), "transf decrypt: %d/%d (running/max)\n", (configuration->maxDecrypt-configuration->decrypt), configuration->maxDecrypt);
			write(fd_clientes[maxClientes], nopAux, strlen(nopAux));
			write(fd_clientes[maxClientes], bcompressAux, strlen(bcompressAux));
			write(fd_clientes[maxClientes], bdecompressAux, strlen(bdecompressAux));
			write(fd_clientes[maxClientes], gcompressAux, strlen(gcompressAux));
			write(fd_clientes[maxClientes], gdecompressAux, strlen(gdecompressAux));
			write(fd_clientes[maxClientes], encryptAux, strlen(encryptAux));
			write(fd_clientes[maxClientes], decryptAux, strlen(decryptAux));
			_exit(0);
		}
	}
	else if(strcmp("FINISH", string) == 0){
		string = strtok(NULL, " ");
		string = strtok(NULL, " ");
		int tar = atoi(string);
		printf("tarefa: %d terminada\n",tar);
		for (int i = 0; i < tarefas[tar-1]->nrTransf; ++i)
		{
			if (!strcmp(tarefas[tar-1]->transf[i], "nop")) configuration->nop++;
			else if (!strcmp(tarefas[tar-1]->transf[i], "bcompress")) configuration->bcompress++;
			else if (!strcmp(tarefas[tar-1]->transf[i], "bdecompress")) configuration->bdecompress++;
			else if (!strcmp(tarefas[tar-1]->transf[i], "gcompress")) configuration->gcompress++;
			else if (!strcmp(tarefas[tar-1]->transf[i], "gdecompress")) configuration->gdecompress++;
			else if (!strcmp(tarefas[tar-1]->transf[i], "encrypt")) configuration->encrypt++;
			else configuration->decrypt++;
		}
		tarefas[tar-1]->status=3;
		if (currentQ>=0){
			printf("entrei\n");
			for (int j = 0; j <= currentQ; ++j){
				int nop=0, bcompress=0, bdecompress=0, gcompress=0, gdecompress=0, encrypt=0, decrypt=0;
				if (temRecursos(stack[j]->transf, stack[j]->nrTransf, &nop, &bcompress, &bdecompress, &gcompress, &gdecompress, &encrypt, &decrypt)){
					current++;
					configuration->nop = configuration->nop - nop;
					configuration->bcompress = configuration->bcompress - bcompress;
					configuration->bdecompress = configuration->bdecompress - bdecompress;
					configuration->gcompress = configuration->gcompress - gcompress;
					configuration->gdecompress = configuration->gdecompress - gdecompress;
					configuration->encrypt = configuration->encrypt - encrypt;
					configuration->decrypt = configuration->decrypt - decrypt;
					tarefas[current] = malloc(1 * sizeof(struct tarefa));
					tarefas[current]=stack[j];
					if (fork()==0){
						write(fd_clientes[tarefas[current]->idCliente], "Processing\n", 11);
						exec_tarefa(stack[j]->transf, stack[j]->nrTransf, stack[j]->inputfile, stack[j]->outputfile);
					}
					else{
						if (currentQ>0){
						for (int z = j; z < currentQ; z++){  
    					stack[z] = stack[z+1]; // assign stack[i+1] to stack[i] 
    					} 
						}
						j--;
    					currentQ--;
					}
				}
			}
		}


	}
	printf("Saí fora do interpretador\n");
	return 1;
}

int main(int argc, char *argv[]){
	char buf[MAX_SIZE];
	int fd_ficticio;
	int bytes_read = 0;
	signal(SIGINT,signIntHandler);


	if((fd_cl_sv_read = open("fifo c->s",O_RDONLY)) == -1){
		perror("open");
		return -1;
	} 
	else
		printf("[DEBUG] opened fifo c->s for [reading]\n");

	// open named pipe for writing
	if((fd_sv_cl_write = open("fifo s->c",O_WRONLY)) == -1){
		perror("open");
		return -1;
	}
	else
		printf("[DEBUG] opened fifo s->c for [writing]\n");

	if((fd_ficticio = open("fifo c->s",O_WRONLY)) == -1){
		perror("open");
		return -1;
	}
	else
		printf("[DEBUG] opened fifo ficticio for [writing]\n");

	if (argc == 3){
		init_Configuration(argv[1], argv[2]);
	}

	else {
		write(1, "Argumentos inválidos\n", 22);
	}

	while((bytes_read = read(fd_cl_sv_read, buf, MAX_SIZE)) > 0){
		buf[bytes_read] = '\0';
		char* input = malloc(strlen(buf) * sizeof(char));
		strcpy(input, buf);
		char* token = strtok(buf, " ");
		if (strcmp(token,"PID")==0){
			token = strtok(NULL, " ");
			printf("%s\n",token );
			int fdfifo = atoi(token);
			token = strtok(NULL, "\0");
			printf("%s\n", token);
			maxClientes++;
			char fifo[MAX_SIZE];
			snprintf(fifo, sizeof(fifo), "%s %d", "fifo", fdfifo);
			int fdCliente;
			if((fdCliente = open(fifo, O_WRONLY))==-1){
				perror("FIFO CLIENTE");
			}
			else
				fd_clientes[maxClientes] = fdCliente;
			interpreter(token);
			bzero(buf, MAX_SIZE * sizeof(char));

		}

		else if(strcmp(token, "FINISH")==0){
			printf("%s\n", input);
			printf("%s\n", token);
			interpreter(input);
		}

		else if (strcmp(buf, "exit") == 0){
			close(fd_ficticio);
			printf("fechei\n");
			close(fd_sv_cl_write);
			close(fd_cl_sv_read);
		}
		free(input);
	}


	
	return 0;
}