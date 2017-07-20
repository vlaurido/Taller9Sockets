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

//Main
int main( int argc, char *argv[]) {
	struct sockaddr_in direccion_servidor;
	int sockfd;
	int puerto;
	int res;
	int f;
	char *archivo;
	char *buffer = malloc(BUFLEN*sizeof(char));

	if(argc != 5){
		printf("Uso: ./cliente <ip> <puerto> <archivo a enviar> <nombre del archivo a guardar>");
		exit(-1);
	}

	puerto = atoi(argv[2]);

	//tomamos el archivo
	archivo = argv[3];

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
	direccion_servidor.sin_addr.s_addr = inet_addr(argv[1]);

	res = inet_pton(AF_INET,argv[1],&direccion_servidor.sin_addr);

	if (res == 0) {
		printf("El segundo argumento no contiene una direccion IP valida\n");
		close(sockfd);
		exit(-1);
	}

	if (connect(sockfd, (struct sockaddr *)&direccion_servidor, sizeof(direccion_servidor)) == -1) {
		printf("Error a la hora de conectarse con el servidor\n");
		close(sockfd);
		exit(-1);
	}

	//En este punto ya tenemos una conexión valida
	printf("Se ha conectado con el servidor: %s\n", (char *)inet_ntoa(direccion_servidor.sin_addr));

	send(sockfd,archivo,strlen(archivo),0);
	printf("Archivo enviado al servidor\n");

	//recibimos la respuesta del servidor
	int rec = recv(sockfd,buffer,BUFLEN,0);
	printf("%d\n",rec);

	f = creat(argv[4],S_IRWXU);
	if (f < 0) {
		printf("Error recibiendo el archivo\n");
		exit(-1);
	} else {
		if ((write(f,buffer,rec)) < 0) {
			printf("Error al guardar\n");
			exit(-1);
		} else 
			printf("El archivo ha sido descargado correctamente\n");
	}
	return 0; 
}
