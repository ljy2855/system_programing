CC = gcc
CFLAGS = -Og
#LDLIBS = -lpthread

myShell : csapp.o myshell.o 
	gcc -o myShell myshell.o csapp.o -lpthread

csapp.o : csapp.h csapp.c 
	gcc -c -o csapp.o csapp.c -lpthread

myshell.o : myshell.h myshell.c csapp.h 
	gcc -c -o myshell.o myshell.c -lpthread
clean:
	rm -rf *.o myShell