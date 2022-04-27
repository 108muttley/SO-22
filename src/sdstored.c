#include "include/sdstore.h"


typedef struct struct_configuration{
	int nop;
	int bcompress;
	int bdecompress; 
	int gcompress;
	int gdecompress;
	int encrypt;
	int decrypt;
	char* path_exec;
}*Configuration;

typedef struct tarefa{
	char* input; //ficheiro de input 
	char* output; // ficheiro de output
	int pidFilho;
	char** transf; //
	int nrTransf; //número de transformações
	int status; //1 - em execução, 2 - pending, 3 - terminada
	int priority; //0 a 5 (sendo 5 máxima prioridade)
}*Tarefa;

Tarefa* tarefas;
int current = -1, maxTarefas = 0;
Configuration configuration;


//lê o ficheiro conf para guardar as informações do máximo de cada transformação e guardar o path para os executáveis
void init_Configuration(char* filename, char* path){

	int read_bytes = 0;
	int i= 0;
	int conf[7];
	char buf[1024];
	char* token;
	configuration = malloc(sizeof(struct struct_configuration));
	tarefas = malloc(30 * sizeof(Tarefa));

	int fileConf_fd = open(filename, O_RDONLY);
	if(fileConf_fd < 0){
		perror("open Configuration File");
		return;
	}

	while((read_bytes = myreadln(fileConf_fd, buf, 1024)) > 0){

		token = strtok(buf, " ");
		printf("%s\n",token);
		token = strtok(NULL, " ");
		conf[i] = atoi(token);
		printf("[DEBUG] token:%d\n", conf[i]);
		i++;
	}
	configuration->nop = conf[0];
	configuration->bcompress = conf[1];
	configuration->bdecompress = conf[2];
	configuration->gcompress = conf[3];
	configuration->gdecompress = conf[4];
	configuration->encrypt = conf[5];
	configuration->decrypt = conf[6];
	configuration->path_exec = malloc(strlen(path)*sizeof(char));
	strcpy(configuration->path_exec,path);
	
	//configuration->path_exec = path;
	printf("Path:%s\n",configuration->path_exec);

	close(fileConf_fd);
}

void procfile(char* transf){
	int filein, fileout;
	char* token;
	char inputFile[30] = "";
	char outputFile[30] = "";
	printf("[DEBUG] Buffer:%s\n",transf);
	token = strtok(transf, " ");
	printf("[DEBUG] token:%s\n", token);
	token = strtok(NULL, " ");
	printf("[DEBUG] token:%s\n", token);
	strcpy(inputFile, token);
	token = strtok(NULL, " ");
	printf("[DEBUG] token:%s\n", token);
	strcpy(outputFile, token);
	filein = open(inputFile, O_RDONLY);
	if(filein < 0){
		perror("open input file");
		return;
	}
	printf("Out: %s\n", outputFile);

	fileout = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0640);
	if(fileout < 0){
		perror("open output file");
		return;
	}
	char buffer[100] = "";
	snprintf( buffer, sizeof(buffer), "%s%s", configuration->path_exec, "/encrypt");
	printf("EXEC: %s\n", buffer);
	dup2(filein,0);
	close(filein);
	dup2(fileout,1);
	close(fileout);
	execv(buffer, NULL);


	return;

}

int exec_command(char* command)
{
	//pid_t pid;
	int exec_ret;
	char buffer[100] = "";
	// First part: parsing
	//char* exec_args[20];
	//char* string;
	//int i;

	/*string = strtok(command, " ");

	for (i = 0; string != NULL; i++) {
		exec_args[i] = string;
		string = strtok(NULL, " ");
	}

	exec_args[i] = NULL;
	*/

	snprintf( buffer, sizeof(buffer), "%s%s%s", configuration->path_exec, "/", command);



	exec_ret = execvp(buffer, NULL);


	return exec_ret;
}

int interpreter(char* input){
	char *string = strtok(input," ");
	char* inputfile, *outfile;
	printf("String:%s\n", string);
	char** comandos;
	comandos = malloc(30 * sizeof(char*));
	int pid=0, nrComandos=0,i, nop=0, bcompress=0, bdecompress=0, gcompress=0, gdecompress=0, encrypt=0, decrypt=0, filein, fileout;
	if (strcmp(string, "proc-file") == 0){
		string = strtok(NULL, " ");
		inputfile = malloc(strlen(string) * sizeof(char));
		strcpy(inputfile, string);
		printf("INPUTFILE: %s\n", inputfile );
		string = strtok(NULL, " ");
		outfile = malloc(strlen(string) * sizeof(char));
		strcpy(outfile, string);
		printf("OUTFILE: %s\n", outfile);
		string = strtok(NULL, " ");
		printf("Comando:%s\n", string );
		for (i = 0; string != NULL; i++) {
			comandos[i] = malloc((strlen(string) * sizeof(char)));
			strcpy(comandos[i], string);
			if (!strcmp(string, "nop")) nop++;
			else if (!strcmp(string, "bcompress")) bcompress++;
			else if (!strcmp(string, "bdecompress")) bdecompress++;
			else if (!strcmp(string, "gcompress")) gcompress++;
			else if (!strcmp(string, "gdecompress")) gdecompress++;
			else if (!strcmp(string, "encrypt")) encrypt++;
			else if (!strcmp(string, "decrypt")) decrypt++;
			else {
				printf("[DEBUG] Tarefa Inválida\n");
				return 0;
			}
			string = strtok(NULL, " \0");
			printf("Comando: %s\n", string);
		}
		printf("terminou\n");
		nrComandos = i;
		printf("i:%d\n", i);
		int pipes[nrComandos-1][2];
		maxTarefas++;
		if (!(((configuration->nop - nop) < 0) || 
			((configuration->bcompress - bcompress) < 0) || 
			((configuration->bdecompress - bdecompress) < 0) ||
			((configuration->gcompress - gcompress) < 0) ||
			((configuration->gdecompress - gdecompress) < 0) ||
			((configuration->encrypt - encrypt) < 0) ||
			((configuration->decrypt - decrypt) < 0))){
			printf("ENTREI\n");
			current++;
			printf("CURRENT: %d\n", current );
			tarefas[current] = malloc(1 * sizeof(struct tarefa));
			printf("a\n");
			tarefas[current]->input = inputfile;
			printf("b\n");
			tarefas[current]->output = outfile;
			printf("c\n");
			tarefas[current]->transf = comandos;
			printf("d\n");
			tarefas[current]->nrTransf = nrComandos;
			printf("e\n");
			tarefas[current]->pidFilho = pid;
			printf("f\n");
			tarefas[current]->priority = 0;
			tarefas[current]->status = 1; //em execução 
			printf("AAA\n");
			if((pid = fork()) == 0){
				//executar comandos
				filein = open(inputfile, O_RDONLY);
				if(filein < 0){
					perror("open input file");
					return -1;
				}
				printf("Out: %s\n", outfile);

				fileout = open(outfile, O_CREAT | O_TRUNC | O_WRONLY, 0640);
				if(fileout < 0){
					perror("open output file");
					return -1;
				}
				for (int c = 0; c < nrComandos; ++c)
				{
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
			}
		}
	}
	return 1;
}

int main(int argc, char *argv[]){
	char buf[2048];
	int bytes_read = 0;


	if (argc == 3){
		init_Configuration(argv[1], argv[2]);
	}

	else {
		write(1, "Argumentos inválidos\n", 22);
	}

	while((bytes_read = read(0, buf, 2048)) > 0){
		buf[bytes_read-1] = '\0';
		printf("bytes_read: %d\n", bytes_read);
		interpreter(buf);
	}


	
	return 0;
}