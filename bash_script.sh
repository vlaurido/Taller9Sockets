#! /bin/bash

for i in  {1..10000}; do
	for  j in {0..49}; do
		./cliente 127.0.0.1 1100 ../C:/Users/Eduardo/Desktop/archivos_aleatorios/archivo$j archivo$j
	done
done

