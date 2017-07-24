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

#define BUFLEN 1024
#define MAXSLEEP 64
#define BUFFERING 100000000

int connect_retry( int domain, int type, int protocol, 	const struct sockaddr *addr, socklen_t alen){
	
	int numsec, fd; /* * Try to connect with exponential backoff. */ 

	for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1) { 

		if (( fd = socket( domain, type, protocol)) < 0) 
			return(-1); 
		if (connect( fd, addr, alen) == 0) { /* * Conexión aceptada. */ 
			return(fd); 
		} 
		close(fd); 				//Si falla conexion cerramos y creamos nuevo socket
		/* * Delay before trying again. */
		if (numsec <= MAXSLEEP/2)
			sleep( numsec); 
	} 
	return(-1); 
}

int main( int argc, char *argv[]) { 
	int sockfd;
	int filefd; 

	if(argc == 1){
		printf("Ingrese con formato: ./cliente [ip] [puerto]\n");
		exit(-1);
	}
	if(argc != 5){
		printf( "Ingreso de puerto obligatorio\n");
	}
	int puerto = atoi(argv[2]);


	struct sockaddr_in conexion_cliente;
	memset(&conexion_cliente, 0, sizeof(conexion_cliente));	//ponemos en 0 la estructura direccion_servidor

	//seteamos los atributos de sockaddr
	conexion_cliente.sin_family = AF_INET;		//IPv4
	conexion_cliente.sin_port = htons(puerto);		//Convertimos el numero de puerto al endianness de la red
	conexion_cliente.sin_addr.s_addr = inet_addr(argv[1]) ;	//Nos tratamos de conectar a esta direccion

	//AF_INET + SOCK_STREAM = TCP
	if (( sockfd = connect_retry( conexion_cliente.sin_family, SOCK_STREAM, 0, (struct sockaddr *)&conexion_cliente, sizeof(conexion_cliente))) < 0) { 
		printf("falló conexión\n"); 
		exit(-1);
	} 
	
	//Aqui ocurrira el recio y envio de informacion con los clientes
	void * buffer1 = malloc(BUFLEN*sizeof(char *));
	if (recv(sockfd,buffer1,1024,0) == -1){
		printf("Error en rcibir, Verifique que exista archivo solicitado \n");
		exit(-1);
	}
	printf(" %s \n",(char*)buffer1);
	char buffer2[BUFLEN]="GET ";
	send(sockfd,strcat(buffer2,argv[3]),BUFLEN,0);
	//procesamos la respuesta del servidor
	int n=0;  	
	char *archivo = malloc(BUFFERING*sizeof(char *));
	memset(archivo,0,BUFFERING);
	filefd = open(argv[4],O_CREAT|O_TRUNC|O_RDWR,S_IRWXU);
	if (filefd < 0){
	    printf("Error al intentar abrir el archivo\n");
	    return -1;
	}
	printf("procesando respuesta del servidor\n");
	while((n=recv(sockfd, archivo, BUFFERING,0))>0){
	    if ((write(filefd,archivo,n)) < 0){
	      	printf("No se puede escribir el archivo. Verifique si fue recibido\n");
	      	return -1;
	    }else{
	      	printf("****Descargando******\n");
	      	memset(archivo,0,BUFFERING);
	    	}
		}
		printf("Se ha descargado archivo solicitado sastifactoriamente\n");
		memset(archivo,0,BUFFERING);
		free(archivo);
		close(filefd);
	return 0;
}