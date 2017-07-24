all: servidor cliente

servidor: servidor_multi.c 
	gcc -Wall servidor_multi.c -o servidor

cliente: cliente_multi.c 
	gcc -Wall cliente_multi.c -o cliente

clean:
	rm -rf cliente servidor archivo*
