#include "csapp.h"
#define NOT_FOUND -1
#define NOT_ENOUGH 0
#define SUCCESS 1
#define MAX_CLIENT 128
#define MAX_FD 1024
typedef struct stock Stock;
struct stock
{
    int ID;
    Stock * left_stock;
    Stock * right_stock;
    int price;
    int cnt;
    sem_t mutex;
};

char  stock_info[MAXBUF];
Stock *root_tree = NULL;


void insert_stock(int ID, int price, int cnt){
    Stock *new_stock = (Stock *)malloc(sizeof(Stock));
    new_stock->ID = ID;
    new_stock->price = price;
    new_stock->cnt = cnt;
    new_stock->left_stock = NULL;
    new_stock->right_stock = NULL;
    
    if (root_tree == NULL)
    {
        root_tree = new_stock;
    }
    else
    {
        Stock *cur = root_tree;
        while(cur != NULL){
    
            if(cur->ID > new_stock->ID){
                // move down tree left
                if(cur->left_stock == NULL){
                    // insert node
                    cur->left_stock = new_stock;
                    break;
                }
                cur = cur->left_stock;
            }else{
                // move donw tree right
                if(cur->right_stock == NULL){
                    cur->right_stock = new_stock;
                    break;
                }
                cur = cur->right_stock;
            }
        }
    }
}

Stock * get_stock(int ID){
    Stock *cur = root_tree;
    while(cur != NULL){
        if(cur->ID == ID){
            break;
        }
        if(cur->ID > ID){
            // move down tree left
            if (cur->left_stock == NULL)
            {
                // can't find
                cur = NULL;
                break;
            }
            cur = cur->left_stock;
        }
        else
        {
            // move down tree right
            if (cur->right_stock == NULL)
            {
                // can't find
                cur = NULL;
                break;
            }
            cur = cur->right_stock;
        }
    }
    return cur;
}



void set_stocks_info(Stock *cur)
{
    if(cur == root_tree){
        stock_info[0] = 0;
    }
    char temp[200];
    if(cur != NULL){
        sprintf(temp, "%d %d %d\n",cur->ID,cur->cnt,cur->price);
        strcat(stock_info, temp);
        set_stocks_info(cur->left_stock);
        set_stocks_info(cur->right_stock);
        
        
    }else{
        // return '\0';
    }

}
void write_to_file()
{
    FILE *fp = fopen("stock.txt", "w");
    fprintf(fp, "%s", stock_info);
    fclose(fp);
}

int update_stock(int ID, int cnt)
{
    Stock *target_stock = get_stock(ID);
    if (target_stock == NULL)
    {
        return NOT_FOUND;
    }
    if (target_stock->cnt + cnt < 0)
    {
        return NOT_ENOUGH;
    }
    target_stock->cnt += cnt;
    set_stocks_info(root_tree);
    return SUCCESS;
}

char * process_request(char command[]){
    char * prefix;
    char * result = (char*) malloc(sizeof(char)* MAXBUF);
    memset(result,0,MAXBUF);
    prefix = strtok(command, " \n");
    if(strcmp(prefix,"show")== 0){
        set_stocks_info(root_tree);
 
        strcpy(result,stock_info);
    }else if(strcmp(prefix, "buy") == 0){
        int id, cnt;
        id = atoi(strtok(NULL," \n"));
        cnt = atoi(strtok(NULL," \n"));
        cnt *= -1;
        switch (update_stock(id,cnt))
        {
        case SUCCESS:
            strcpy(result,"[buy] \033[0;32msuccess\033[0m\n");
            break;
        
        case NOT_FOUND:
            strcpy(result,"Stock Not found\n");
            break;
        case NOT_ENOUGH:
            strcpy(result,"Not enough left stock\n");
            break;
        }


    }else if(strcmp(prefix, "sell") == 0 ){
        int id, cnt;
        id = atoi(strtok(NULL," \n"));
        cnt = atoi(strtok(NULL," \n"));
        switch (update_stock(id,cnt))
        {
        case SUCCESS:
            strcpy(result,"[buy] \033[0;32msuccess\033[0m\n");
            break;
        
        case NOT_FOUND:
            strcpy(result,"Stock Not found\n");
            break;
        }
    }
    return result;
}

void terminate_handler(int sig){
    printf("write to file...\n");
    write_to_file();
    printf("write file success\n");
    printf("server close\n");
    exit(0);
}


int init_stock(){
    FILE *fp = fopen("stock.txt", "r");
    while(!feof(fp)){
        int id, cnt, price;
        if(fscanf(fp,"%d %d %d", &id, &cnt, &price)>0)
            insert_stock(id, price, cnt);
    }
    set_stocks_info(root_tree);
    //printf("%s\n", stock_info);
    fclose(fp);
    return 1;
}

