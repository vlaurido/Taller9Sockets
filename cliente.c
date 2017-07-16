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


#define BUFLEN 128 
#define MAXSLEEP 64

void sendFileName(int sockfd, char name[80]) {
	printf("Nombre enviado: %s\n", name);
	if (write(sockfd,name,strlen(name)) == -1)
		printf("Error al enviar la confirmacion\n");
}

int main( int argc, char *argv[]) {
	struct sockaddr_in direccion_servidor;
	int socksize;
	int sockfd;
	int puerto;
	int res;
	char mensaje[80];
	char buffer[BUFLEN];
	FILE *archivo;

	if(argc != 5){
		printf("Uso: ./cliente <ip> <puerto> <archivo a enviar> <nombre del archivo a guardar>");
		exit(-1);
	}

	puerto = atoi(argv[2]);

	//abrimos el archivo
	archivo = fopen(argv[3],"rb");
	if (!archivo) {
		printf("Error al abrir el archivo\n");
		exit(-1);
	}

	//creamos el socket
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//configuramos la direccion del socket
	memset(&direccion_servidor, 0, sizeof(direccion_servidor));	//ponemos en 0 la estructura direccion_servidor

	//llenamos los campos
	direccion_servidor.sin_family = AF_INET;		//IPv4
	direccion_servidor.sin_port = htons(puerto);		//Convertimos el numero de puerto al endianness de la red
	res = inet_pton(AF_INET,argv[1],&direccion_servidor.sin_addr);
	socksize = sizeof(direccion_servidor);

	if (res == 0) {
		printf("El segundo argumento no contiene una direccion IP valida\n");
		close(sockfd);
		exit(-1);
	}

	if (connect(sockfd, (struct sockaddr *)&direccion_servidor, sizeof(direccion_servidor)) == -1) {
		printf("Error a la hora de conectarse con el cliente\n");
		close(sockfd);
		exit(-1);
	}

	//En este punto ya tenemos una conexión valida
	printf("Se ha conectado con el servidor: %s\n", (char *)inet_ntoa(direccion_servidor.sin_addr));

	while(!feof(archivo))  {
		fread(buffer,sizeof(char),BUFLEN,archivo);
		//sendFileName(sockfd,argv[4]);
		if (send(sockfd,buffer,BUFLEN,0) == -1)
			printf("Error al enviar el archivo\n");
	}

	read(sockfd,mensaje,sizeof(mensaje));
	printf("Confirmacion recibida:\n%s\n", mensaje);

	return 0; 
}


