/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"

void init_stock();
void * connect_client(void *arg);
char * process_request(char command[]);

void terminate_handler(int sig);
int main(int argc, char **argv)
{
    int listenfd,* connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t t_id;

    Signal(SIGINT,terminate_handler);

    init_stock();
    

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

    listenfd = Open_listenfd(argv[1]);


    while (1){
   
        connfd = (int*)malloc(sizeof(int));
        clientlen = sizeof(struct sockaddr_storage);
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        pthread_create(&t_id,NULL,connect_client,connfd);

    }
    
    exit(0);
}
void * connect_client(void *arg){
    int connfd = *((int*) arg);
    int n; 
    char buf[MAXLINE]; 
    char * response;
    rio_t rio;
    pthread_detach(pthread_self());
    

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", n);
        
        response = process_request(buf);
        Rio_writen(connfd, response, MAXLINE);
        free(response);
    }
    close(connfd);
    Free(arg);
    return NULL;
}


/* $end echoserverimain */
