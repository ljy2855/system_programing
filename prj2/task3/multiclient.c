#include "csapp.h"
#include <time.h>
#include <sys/time.h>

#define MAX_CLIENT 500
#define ORDER_PER_CLIENT 100
#define STOCK_NUM 10
#define BUY_SELL_MAX 10
#define SET_SHOW 0
#define SET_BUY 1
#define SET_SELL 2
#define SET_RANDOM() (rand() % 3)
int main(int argc, char **argv) 
{
	pid_t pids[MAX_CLIENT];
	
	int runprocess = 0, status, i;
	struct timeval tv;
	double begin, end;

	int clientfd, num_client;
	char *host, *port, buf[MAXLINE], tmp[3];
	rio_t rio;
	int file_fd = open("result.md", O_RDWR| O_APPEND);
	

	if (argc != 4)
	{
		fprintf(stderr, "usage: %s <host> <port> <client#>\n", argv[0]);
		exit(0);
	}

	host = argv[1];
	port = argv[2];
	num_client = atoi(argv[3]);
	gettimeofday(&tv, NULL);
	begin = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
	sprintf(buf, "client :%d, request:%d\n", num_client, ORDER_PER_CLIENT);
	Rio_writen(file_fd, buf, strlen(buf));

	/*	fork for each client process	*/
	while(runprocess < num_client){
		//wait(&state);
		pids[runprocess] = fork();

		if(pids[runprocess] < 0)
			return -1;
		/*	child process		*/
		else if(pids[runprocess] == 0){
			//printf("child %ld\n", (long)getpid());

			clientfd = Open_clientfd(host, port);
			Rio_readinitb(&rio, clientfd);
			srand((unsigned int) getpid());

			for(i=0;i<ORDER_PER_CLIENT;i++){
				//int option = rand() % 3;
				int option = SET_RANDOM();
				if(option == 0){//show
					strcpy(buf, "show\n");
				}
				else if(option == 1){//buy
					int list_num = rand() % STOCK_NUM + 1;
					int num_to_buy = rand() % BUY_SELL_MAX + 1;//1~10

					strcpy(buf, "buy ");
					sprintf(tmp, "%d", list_num);
					strcat(buf, tmp);
					strcat(buf, " ");
					sprintf(tmp, "%d", num_to_buy);
					strcat(buf, tmp);
					strcat(buf, "\n");
				}
				else if(option == 2){//sell
					int list_num = rand() % STOCK_NUM + 1; 
					int num_to_sell = rand() % BUY_SELL_MAX + 1;//1~10
					
					strcpy(buf, "sell ");
					sprintf(tmp, "%d", list_num);
					strcat(buf, tmp);
					strcat(buf, " ");
					sprintf(tmp, "%d", num_to_sell);
					strcat(buf, tmp);
					strcat(buf, "\n");
				}
				//strcpy(buf, "buy 1 2\n");
			
				Rio_writen(clientfd, buf, strlen(buf));
				//Rio_writen(STDOUT_FILENO, buf, strlen(buf));
				// Rio_readlineb(&rio, buf, MAXLINE);
				Rio_readnb(&rio, buf, MAXLINE);
				//Fputs(buf, stdout);

				usleep(100000);
			}
			gettimeofday(&tv, NULL);
			end = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

			// 출력
			
			sprintf(buf, "%f\n", (end - begin) / 1000);
			Rio_writen(file_fd, buf, strlen(buf));

			Close(clientfd);
			exit(0);
		}
		/*	parten process		*/
		/*else{
			for(i=0;i<num_client;i++){
				waitpid(pids[i], &status, 0);
			}
		}*/
		runprocess++;
	}
	for(i=0;i<num_client;i++){
		waitpid(pids[i], &status, 0);
	}
	Rio_writen(file_fd, "\n", 1);

	/*clientfd = Open_clientfd(host, port);
	Rio_readinitb(&rio, clientfd);

	while (Fgets(buf, MAXLINE, stdin) != NULL) {
		Rio_writen(clientfd, buf, strlen(buf));
		Rio_readlineb(&rio, buf, MAXLINE);
		Fputs(buf, stdout);
	}

	Close(clientfd); //line:netp:echoclient:close
	exit(0);*/

	return 0;
}
