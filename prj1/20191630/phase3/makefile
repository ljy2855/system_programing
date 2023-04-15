CC = gcc

myshell : myshell.o
	gcc -o myshell myshell.o
myshell.o : myshell.c
	gcc -c -o myshell.o myshell.c

clean :
	rm *.o myshell .bash_history