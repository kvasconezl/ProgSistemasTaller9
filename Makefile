all: servidor cliente

servidorT: servidor.c 
	gcc -Wall -g $^ -o $@

clienteT: cliente.c 
	gcc -Wall $^ -o $@

clean:
	rm -rf cliente servidor
