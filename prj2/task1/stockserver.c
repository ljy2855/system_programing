/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"


typedef struct{
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    rio_t rio[FD_SETSIZE];
} pool;


void echo(int connfd);
void init_stock();
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
char * process_request(char command[]);
void close_client(int connfd,pool *p,int i);
void check_clients(pool *p);
void terminate_handler(int sig);
int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    static pool pool;
    Signal(SIGINT,terminate_handler);

    init_stock();

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd,&pool);

    while (1){
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);
        if(FD_ISSET(listenfd,&pool.ready_set)){
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                        client_port, MAXLINE, 0);
            printf("Connected to (%s, %s)\n", client_hostname, client_port);
            add_client(connfd, &pool);
        }
        check_clients(&pool);
        
    }
    
    exit(0);
}

void init_pool(int listenfd, pool *p){
    p->maxi = -1;
    memset(p->clientfd,-1,sizeof(int)*FD_SETSIZE);
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd,&p->read_set);
}

void add_client(int connfd, pool *p){
    int i;
    p->nready--;
    for(i=0; i < FD_SETSIZE; i++){
        if(p->clientfd[i] < 0){
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->rio[i],connfd);
            FD_SET(connfd,&p->read_set);

            if(connfd > p->maxfd)
                p->maxfd = connfd;
            if (i > p->maxi)
                p->maxi = i;
            break;
        }
    }
    if(i == FD_SETSIZE){
        app_error("cant accept more clients");
    }
}

void check_clients(pool *p){
    int i, connfd, n;
    char buf[MAXBUF];
    char * response;
    rio_t rio;
    for(i = 0; (i <= p->maxi) && (p->nready > 0); i++){
        connfd  = p->clientfd[i];
        rio = p->rio[i];
        if((connfd >0) && (FD_ISSET(connfd, &p->ready_set))){
            p->nready--;
            memset(buf,0,MAXBUF);
            if((n = Rio_readlineb(&rio,buf,MAXBUF)) != 0){
                // get data from client
                printf("server received %d bytes\n", n);
                if(strcmp(buf,"exit\n") == 0){
                    Rio_writen(connfd,buf,n);
                    Close(connfd);
                    FD_CLR(connfd,&p->read_set);
                    p->clientfd[i]=-1;
                    printf("client disconnect\n");
                    continue;
                }else if(strcmp(buf,"\n")== 0)
                    continue;
                response = process_request(buf);
                Rio_writen(connfd,response,MAXLINE);
                free(response);

            }else{
                // EOF detected
                Close(connfd);
                FD_CLR(connfd,&p->read_set);
                p->clientfd[i]=-1;
            }
        }
    }
}

void close_client(int connfd,pool *p,int i){
    Close(connfd);
    FD_CLR(connfd,&p->read_set);
    p->clientfd[i]=-1;
}
/* $end echoserverimain */
