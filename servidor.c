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
#define QLEN 10 

#ifndef HOST_NAME_MAX 
#define HOST_NAME_MAX 256 
#endif	

//Funcion para enviar un mensaje de confirmacion
void sendOk(int sockfd) {
	char mensaje[80] = "Archivo recibido";
	printf("Confirmacion enviada\n");
	if (write(sockfd,mensaje,sizeof(mensaje)) == -1)
		printf("Error al enviar la confirmacion\n");
}

//Main
int main( int argc, char *argv[]) {
	struct sockaddr_in direccion_servidor;
	struct sockaddr_in direccion_cliente;
	FILE *fp;
	char *host;
	char buf[BUFLEN];
	int puerto;
	int sockfd;
	int clfd;
	int clsize;
	int recibido = -1;
	int n;

	if(argc == 1){
		printf("Uso: ./servidor <ip> <numero de puerto>\n");
		exit(-1);
	}

	if(argc != 3){
		printf( "Por favor especificar un numero de puerto\n");
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
	sockfd = socket(((struct sockaddr*)&direccion_servidor)->sa_family, SOCK_STREAM, IPPROTO_TCP);

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

	while(1){
		clsize = sizeof(direccion_cliente);

		if ((clfd = accept(sockfd, 
					(struct sockaddr *) &direccion_cliente,
					&clsize)) < 0) { 		//Aceptamos una conexion
			syslog( LOG_ERR, "ruptimed: accept error: %s", strerror( errno)); 	//si hay error la ponemos en la bitacora			
			exit( 1); 
		}

		//configuramos la direccion del cliente
		direccion_cliente.sin_family = AF_INET;
		direccion_cliente.sin_port = htons(puerto);
		printf("Cliente conectado: %s\n",inet_ntoa(direccion_cliente.sin_addr));

		if ((fp = fopen("archivoRecibido", "wb")) == NULL) { 		//llamamos al programa uptime con un pipe
			sprintf( buf, "error: %s\n", strerror(errno)); 
			send( clfd, buf, strlen( buf), 0); 
		} 
		else {
			sendOk(sockfd);
			while ((recibido = recv(sockfd,buf,BUFLEN,0)) > 0) {
				printf("%s\n",buf);
				fwrite(buf,sizeof(char),1,fp);
			}
			fclose(fp);
		} 		
	}
	close(clfd);
	close(sockfd);
	return 0;
}