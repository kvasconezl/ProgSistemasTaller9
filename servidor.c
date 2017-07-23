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
#define BUFFERING 100000000
#define QLEN 50 

#ifndef HOST_NAME_MAX 
#define HOST_NAME_MAX 256 
#endif	

//Funcion de ayuda para setear la bandera close on exec
void set_cloexec(int fd){
	if(fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC) < 0){
		printf("error al establecer la bandera FD_CLOEXEC\n");	
	}
}

//Funcion para inicializar el servidor
int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen){
	int fd;
	int err = 0;
	if((fd = socket(addr->sa_family, type, 0)) < 0)
		return -1;
	if(bind(fd, addr, alen) < 0)
		goto errout;
	if(type == SOCK_STREAM || type == SOCK_SEQPACKET){
		if(listen(fd, QLEN) < 0)
			goto errout;
	}
	return fd;
errout:
	err = errno;
	close(fd);
	errno = err;
	return (-1);
}

//Damos el servicio
void serve(int sockfd) { 
	int acepta_conexion;  
	int fichero_archivo;
	set_cloexec( sockfd); 
//Ciclo para enviar y recibir mensajes
	for (;;) { 
		if (( acepta_conexion = accept( sockfd, NULL, NULL)) < 0) { 		//Se acepta la conexion
			syslog( LOG_ERR, "ruptimed: accept error: %s", strerror( errno)); 	//si hay error la ponemos en la bitacora			
			exit( 1); 
		} 
		set_cloexec(acepta_conexion);

		//Enviar mensaje 
		char envio[BUFLEN]; //Aqui guardaremos el mensaje que enviaremos
		printf("\n*******SE HA INICIADO CON EXITO*******\n");
		printf("|||||----CLIENTE CONECTADO---||||\n");
		strcpy(envio,"SE HA CONECTADO AL SERVIDOR...");
		send(acepta_conexion, envio, BUFLEN,0);

	    char *ruta = malloc(BUFLEN*sizeof(char *));
	    char *file = malloc(BUFFERING*sizeof(char *));
		memset(file,0,BUFFERING);//Se inicicializa en cero todo
		int n=0;
	  	while((n=recv(acepta_conexion, ruta, BUFLEN, 0))==0);//Hasta que reciba todo los bytes
	  	printf("%s\n",ruta);
            
	    fichero_archivo = open(ruta+4,O_RDONLY);//Se busca el archicvo y se abre
	    if (fichero_archivo < 0){//en caso de que no se pueda encontrar 
			printf("Error en archivo\n");
			char * ermjs = "Error en archivo\n";
			send(acepta_conexion, ermjs, strlen(ermjs) ,0);//Enviamos mensaje de error
			return ;
	    }else{
	    	printf("Archivo encontrado y abierto correctamente\n");
	    	int filesize ;
	    	while((filesize= read(fichero_archivo, file, BUFFERING))>0){
		        if ((send(acepta_conexion, file, filesize,0)) <= 0){
			        printf("Error con el archivo\n");
			        char * ermjs = "Error en el envio del archivo\n";
			        send(acepta_conexion, ermjs, strlen(ermjs) ,0);
			        return;
	        	}else{
	        		memset(file,0,BUFFERING);//llenamos de cero el buffer 
	        		printf("enviando......\n");
	        	}
	        }
	        printf("Archivo enviado correctamente\n");
	    }
	    close(fichero_archivo);
	    free(file);
	    close(acepta_conexion); 
	    		//cerramos la conexion con el cliente.	   

	 }
	}	
    


//Main

int main( int argc, char *argv[]) { 
	int sockfd, n;
	char *host; 

	if(argc == 1){
		printf("Uso: ./servidor <numero de puerto>\n");
		exit(-1);
	}
	


	int puerto = atoi(argv[2]);

	if (( n = sysconf(_SC_HOST_NAME_MAX)) < 0) 		
		n = HOST_NAME_MAX; /* best guess */ 
	if ((host = malloc(n)) == NULL) 
		printf(" malloc error"); 
	if (gethostname( host, n) < 0) 		//Obtenemos nombre del host
		printf(" gethostname error"); 
	
	printf("Nombre del host: %s\n", host);	//Mostramos nuestro nombre
	
	//Direccion del servidor
	struct sockaddr_in direccion_servidor;
	memset(&direccion_servidor, 0, sizeof(direccion_servidor));	//ponemos en 0 la estructura direccion_servidor

	//llenamos los campos
	direccion_servidor.sin_family = AF_INET;		//IPv4
	direccion_servidor.sin_port = htons(puerto);		//Convertimos el numero de puerto al endianness de la red
	direccion_servidor.sin_addr.s_addr = inet_addr(argv[1]) ;	//Nos vinculamos a la interface localhost o podemos usar INADDR_ANY para ligarnos A TODAS las interfaces

	//inicalizamos servidor (AF_INET + SOCK_STREAM = TCP)
	if( (sockfd = initserver(SOCK_STREAM, (struct sockaddr *)&direccion_servidor, sizeof(direccion_servidor), 1000)) < 0){	//Hasta 1000 solicitudes en cola 
		printf("Error al inicializar el servidor\n");	
	}		

	while(1){
		serve(sockfd);
		//TODO servimos
	}
	
	exit( 1); 
}

