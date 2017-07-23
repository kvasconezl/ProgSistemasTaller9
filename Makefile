all: servidor cliente

servidor: servidor.c 
	gcc -Wall servidor.c -o servidor

cliente: cliente.c 
	gcc -Wall cliente.c -o cliente

clean:
	rm -rf cliente servidor
