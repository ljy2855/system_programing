CC = gcc
CFLAGS = -Og
#LDLIBS = -lpthread

myShell :  myshell.o 
	gcc -o myShell myshell.o

myshell.o : myshell.h myshell.c 
	gcc -c -o myshell.o myshell.c
clean:
	rm -rf *.o myShell