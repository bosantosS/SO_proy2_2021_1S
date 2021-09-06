#include <getopt.h>
#include <limits.h>
#include <sys/wait.h>
#include <pthread.h>
#include "include/common.h"
#define MAX_FD 2
#include <stdbool.h>

bool dflag=false;

void atender_cliente(int connfd);

void print_help(char *command)
{
	printf("Servidor simple de ejecución remota de comandos.\n");
	printf("uso:\n %s [-d] <puerto>\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
	printf(" -d\t\t\tFunciona en modo daemon\n");
	
}

/* Thread routine */
void *thread(void *vargp){
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self());
	free(vargp);
	atender_cliente(connfd);
	close(connfd);
	return NULL;
}

/**
 * Función que crea argv separando una cadena de caracteres en
 * "tokens" delimitados por la cadena de caracteres delim.
 *
 * @param linea Cadena de caracteres a separar en tokens.
 * @param delim Cadena de caracteres a usar como delimitador.
 *
 * @return Puntero a argv en el heap, es necesario liberar esto después de uso.
 *	Retorna NULL si linea está vacía.
 */
char **parse_comando(char *linea, char *delim)
{
	char *token, *saveptr, *saveptr2;
	char *linea_copy;
	int i, num_tokens = 0;
	char **argv = NULL;

	linea_copy = (char *) malloc(strlen(linea) + 1);
	strcpy(linea_copy, linea);

	/* Obtiene un conteo del número de argumentos */
	token = strtok_r(linea_copy, delim,&saveptr);
	/* recorre todos los tokens */
	while( token != NULL ) {
		token = strtok_r(NULL, delim,&saveptr);
		num_tokens++;
	}
	free(linea_copy);
        
	/* Crea argv en el heap, extrae y copia los argumentos */
	if(num_tokens > 0){

		/* Crea el arreglo argv */
		argv = (char **) malloc((num_tokens + 1) * sizeof(char **));

		/* obtiene el primer token */
		token = strtok_r(linea, delim,&saveptr2);
		/* recorre todos los tokens */
		for(i = 0; i < num_tokens; i++){
			argv[i] = (char *) malloc(strlen(token)+1);
			strcpy(argv[i], token);
			token = strtok_r(NULL, delim,&saveptr2);
		}
		argv[i] = NULL;
	}

	return argv;
}

/**
 * Recoge hijos zombies...
 */
void recoger_hijos(int signal){
	while(waitpid(-1, 0, WNOHANG) >0)
		;

	return;
}


int main(int argc, char **argv)
{
	int opt;

	//Sockets
	int listenfd, *connfd;
	unsigned int clientlen;
	//Direcciones y puertos
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char *haddrp, *port;
	pthread_t tid;

	while ((opt = getopt (argc, argv, "hd")) != -1){
		switch(opt)
		{
			case 'd':
				dflag=true;
				break;
			case 'h':
				print_help(argv[0]);
				return 0;
			default:
				fprintf(stderr, "uso: %s [-d] <puerto>\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				return -1;
		}
	}

	if(!(argc == 2 || argc == 3)){
		fprintf(stderr, "uso: %s [-d] <puerto>\n", argv[0]);
		fprintf(stderr, "     %s -h\n", argv[0]);
		return -1;
	}else{
		// Obtiene el puerto
		int index;
		for (index = optind; index < argc; index++)
			port = argv[index];
	}
	//Valida el puerto
	int port_n = atoi(port);
	if(port_n <= 0 || port_n > USHRT_MAX){
		fprintf(stderr, "Puerto: %s invalido. Ingrese un número entre 1 y %d.\n", port, USHRT_MAX);
		return -1;
	}

	//Registra funcion para recoger hijos zombies
	signal(SIGCHLD, recoger_hijos);

	//Abre un socket de escucha en port
	listenfd = open_listenfd(port);

	if(listenfd < 0)
		connection_error(listenfd);

	 if(dflag){
                //Transformando a daemon
                int i, fd0, fd1, fd2;
                pid_t pid;

                //Liberando el pront
	        if ((pid = fork()) < 0)
		        //err_quit("%s: can't fork", cmd);
		        printf("error\n");
	        else if (pid != 0) /* parent */
		        exit(0);
	        setsid();
	        for (i = 0; i < MAX_FD; i++)
		        close(i);
                //STDIN STDOUT STDERRApuntando a null
	        fd0 = open("/dev/null");
	        fd1 = dup(0);
	        fd2 = dup(0);
        }

        printf("server escuchando en puerto %s...\n", port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = malloc(sizeof(int));
		*connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

		/* Determine the domain name and IP address of the client */
		hp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
						sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		haddrp = inet_ntoa(clientaddr.sin_addr);

		printf("server conectado a %s (%s)\n", hp->h_name, haddrp);
		pthread_create(&tid, NULL, thread, connfd);
		
	}
}

void atender_cliente(int connfd)
{
	int n, status;
	char buf[MAXLINE] = {0};
	char **argv;
	pid_t pid;

	//Comunicación con cliente es delimitada con '\0'
	while(1){
		n = read(connfd, buf, MAXLINE);
		if(n <= 0)
			return;

		printf("Recibido: %s", buf);

		//Detecta "CHAO" y se desconecta del cliente
		if(strcmp(buf, "CHAO\n") == 0){
			write(connfd, "BYE\n", 5);
			int pid_current;
			pid_current = fork();
			if (pid_current < 0) { 
				fprintf(stderr, "Fork Failed");
			
			} else if (pid == 0) { 
			// child process
			/*
/c_sms -a "ACee28fea35ad452b3f809ffd1cb49ede6" 
-s "9a9715333490c1eea7136d17b903a789" 
-t "+593989884022" -f "+14158133502" 
-m "Hello, BOSS"
			*/
				char* command = "./c_sms";
    				char* argument_list[] = {
    				"./c_sms",
    				"-a","ACee28fea35ad452b3f809ffd1cb49ede6",
    				"-s","9a9715333490c1eea7136d17b903a789",
    				"-t","+593989884022","-f","+14158133502",
    				"-m","Hello, el cliente se ha desconectado!!!",
    				NULL};
				if(execvp(command,argument_list) < 0){
					fprintf(stderr, "ERROR>> Al enviar la notificacion!!!\n");
					exit(1);
				}

   			} else { 
   			// parent process
		        	wait(NULL);// parent will wait for the child to complete
       	
         			printf("Child Complete \n");
   			}
		}

		//Remueve el salto de linea antes de extraer los tokens
		buf[n - 1] = '\0';

		//Crea argv con los argumentos en buf, asume separación por espacio
		argv = parse_comando(buf, " ");

		if(argv){
			if((pid = fork()) == 0){
				dup2(connfd, 1); //Redirecciona STDOUT al socket
				dup2(connfd, 2); //Redirecciona STDERR al socket
				if(execvp(argv[0], argv) < 0){
					fprintf(stderr, "Comando desconocido...\n");
					exit(1);
				}
			}

			//Espera a que el proceso hijo termine su ejecución
			waitpid(pid, &status, 0);

			if(!WIFEXITED(status))
				write(connfd, "ERROR\n",7);
			else
				write(connfd, "\0", 1); //Envia caracter null para notificar fin

			/*Libera argv y su contenido
			para evitar fugas de memoria */
			for(int i = 0; argv[i]; i++)
				free(argv[i]);
			free(argv);

		}else{
			strcpy(buf, "Comando vacío...\n");
			write(connfd, buf, strlen(buf) + 1);
		}

		memset(buf, 0, MAXLINE); //Encera el buffer
	}
}