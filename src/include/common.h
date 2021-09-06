#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

//#include "../include/sha256.h"
//#include "../include/uECC.h"
//#include "../include/blowfish.h"
//#include "../include/csapp.h"


#define	MAX_PUBLIC_KEY 64
#define	MAX_PRIVATE_KEY	 21
#define	MAX_SECRET 32 
#define	MAXLINE	 8192  /* Max text line length */

 
 ///////////////////////////////////////
/* Funciones de gestión de conexión */

/**
 * Open and return a listening socket on port. This function is reentrant and protocol-independent.
 *
 * @param port Character array with TCP port, decimal.
 *
 * @return A listening socker. On error, returns -1 and sets errno.
 */
int open_listenfd(char *port);


/**
 * Open connection to server at <hostname, port> and 
 * return a socket descriptor ready for reading and writing. This
 * function is reentrant and protocol-independent.
 *
 * @param hostname Character array with IP address or hostname.
 * @param port Character array with TCP port, decimal.
 *
 * @return Socket file descriptor. On error, returns -1 and sets errno.
 */
int open_clientfd(char *hostname, char *port);

/**
 * Closes the socket, prints error on STDERR and exits.
 *
 * @param connfd Socket file descriptor.
 */
void connection_error(int connfd);