#include <sys/types.h>          /* some systems still require this */
#include <sys/stat.h>
#include <stdio.h>              /* for convenience */
#include <stdlib.h>             /* for convenience */
#include <stddef.h>             /* for offsetof */
#include <string.h>             /* for convenience */
#include <unistd.h>             /* for convenience */
#include <signal.h>             /* for SIG_ERR */ 
#include <netdb.h> 
#include <errno.h> 
#include <syslog.h> 
#include <sys/socket.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#define BUFLEN 1000000
#define QLEN 10 

#ifndef HOST_NAME_MAX 
#define HOST_NAME_MAX 256 
#endif

int sockfd;
int clfd;

//Funcion llamada por signal
void salida(int sig) {
	if (sig == SIGTSTP) {
		close(sockfd);
		close(clfd);
		printf("\tLos sockets han sido cerrados\n");
		printf("\tUsted ha elegido terminar el proceso\n");
		exit(0);
	}
}

//Main
int main( int argc, char *argv[]) {
	struct sockaddr_in direccion_servidor;
	struct sockaddr_in direccion_cliente;
	void *file = malloc(BUFLEN);
	char *host;
	char *buf = malloc(BUFLEN);
	int f;
	int size;
	int puerto;
	unsigned int clsize;
	int n;

	if(argc != 3){
		printf("Uso: ./servidor <ip> <numero de puerto>\n");
		exit(-1);
	}

	puerto = atoi(argv[2]);

	if (( n = sysconf(_SC_HOST_NAME_MAX)) < 0) 		
		n = HOST_NAME_MAX; /* best guess */ 
	if ((host = malloc(n)) == NULL) 
		printf(" malloc error"); 
	if (gethostname( host, n) < 0) 		//Obtenemos nombre del host
		printf(" gethostname error"); 
	
	printf("Nombre del host: %s\n", host);	//Mostramos nuestro nombre

	//creamos el socket
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		printf("Error al abrir el socket\n");
		exit(-1);
	}

	//configuramos la direccion del socket
	memset(&direccion_servidor, 0, sizeof(direccion_servidor));	//ponemos en 0 la estructura direccion_servidor
	
	//llenamos los campos
	direccion_servidor.sin_family = AF_INET;		//IPv4
	direccion_servidor.sin_port = htons(puerto);		//Convertimos el numero de puerto al endianness de la red
	direccion_servidor.sin_addr.s_addr = INADDR_ANY;	//Nos vinculamos a la interface localhost o podemos usar INADDR_ANY para ligarnos A TODAS las interfaces
	
	bind(sockfd, (struct sockaddr*)&direccion_servidor, sizeof(direccion_servidor));
	inet_pton(AF_INET,argv[1],&direccion_servidor.sin_addr);
	printf("Socket atado a la direccion %s\n", (char *)inet_ntoa(direccion_servidor.sin_addr));
	listen(sockfd, 100);

	memset(&direccion_cliente, 0, sizeof(direccion_cliente));
	clsize = sizeof(direccion_cliente);

	//metodo que escucha la senal de ^Z
	signal(SIGTSTP,salida);

	while(1) {
		
		clfd = accept(sockfd,(struct sockaddr *)&direccion_cliente,&clsize);
		
		int pd = fork();

		if (pd==0){

			recv(clfd,buf,BUFLEN,0);

			printf("Cliente conectado: %s\n",inet_ntoa(direccion_cliente.sin_addr));

			f = open(buf, O_RDONLY);

			if (f < 0) {
				printf("Error abriendo el archivo\n");
				char *mensaje = "Error abriendo el archivo\n";
				send(clfd,mensaje,strlen(mensaje),0);
				exit(-1);
			} else {
				size = read(f,file,BUFLEN);
				printf("size: %d\n", size);
				if (size <= 0) {
					printf("Error en la lectura del archivo\n");
					char *mensaje = "Error con el archivo\n";
					send(clfd,mensaje,strlen(mensaje),0);
					exit(-1);
				} else {
					printf("El archivo se leyo correctamente\n");
					if ((write(clfd,file,size)) <= 0) {
						printf("Error con el archivo\n");
						char *mensaje = "Error en el envio del archivo\n";
						send(clfd,mensaje,strlen(mensaje),0);
						exit(-1);
					}
					else
						printf("Archivo enviado correctamente\n");
				}
			}
			close(clfd);
			close(sockfd);
		}
		else{
			close(clfd);
			continue;
		}
	}
	return 0;
}