all: cliente servidor servidorMultiproceso 

cliente: cliente.c 
	gcc -Wall $^ -o $@

servidor: servidor.c 
	gcc -Wall -g $^ -o $@

servidorMultiproceso: servidor_multiproceso.c 
	gcc -Wall -g $^ -o $@

clean:
	rm -rf cliente servidor servidorMultiproceso
