all: Grafo.o main.o
	gcc Grafo.o main.o -o main -std=c99 -Wall

Grafo.o:
	gcc -c Grafo.c -o Grafo.o

main.o:
	gcc -c main.c -o main.o

clean:
	-rm *.o main

run:
	./main < case1.in


