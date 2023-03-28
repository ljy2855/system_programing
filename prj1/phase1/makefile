CC = gcc

myShell : myshell.o
	gcc -o myShell myshell.o
myshell.o : myshell.c
	gcc -c -o myshell.o myshell.c

clean :
	rm *.o myShell