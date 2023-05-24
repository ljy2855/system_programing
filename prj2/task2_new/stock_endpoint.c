#include "csapp.h"
#define NOT_FOUND -1
#define NOT_ENOUGH 0
#define SUCCESS 1
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

Stock *root_tree = NULL;

void insert_stock(int ID, int price, int cnt){
    Stock *new_stock = (Stock *)malloc(sizeof(Stock));
    Sem_init(&new_stock->mutex,0,1);
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
        if (cur->ID == ID)
        {
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



void set_stocks_info(Stock *cur,char stock_info[])
{
  
    
    char temp[200];
    if(cur != NULL){
        P(&cur->mutex);
        sprintf(temp, "%d %d %d\n",cur->ID,cur->cnt,cur->price);
        strcat(stock_info, temp);
        
        set_stocks_info(cur->left_stock,stock_info);
        set_stocks_info(cur->right_stock,stock_info);
        V(&cur->mutex);
    }
    
}
void write_to_file()
{
    char *result = (char *)malloc(sizeof(char) * MAXBUF);
    memset(result,0,MAXBUF);
    set_stocks_info(root_tree,result);
    FILE *fp = fopen("stock.txt", "w");
    fprintf(fp,"%s",result);
    free(result);
    fclose(fp);
}

int update_stock(int ID, int cnt)
{
    Stock *target_stock = get_stock(ID);
    P(&target_stock->mutex);
    if (target_stock == NULL)
    {
        V(&target_stock->mutex);
        return NOT_FOUND;
    }
    if (target_stock->cnt + cnt < 0)
    {
        V(&target_stock->mutex);
        return NOT_ENOUGH;
    }
    
    target_stock->cnt += cnt;
    V(&target_stock->mutex);
    return SUCCESS;
}

char * process_request(char command[]){
    
    char *prefix;
    char *next;
    char *result = (char *)malloc(sizeof(char) * MAXBUF);

    memset(result,0,MAXBUF);
    prefix = strtok_r(command, " \n",&next);
    if(strcmp(prefix,"show")== 0){
       
        set_stocks_info(root_tree, result);
        
    }
    else if (strcmp(prefix, "buy") == 0)
    {
        int id, cnt;
        id = atoi(strtok_r(NULL," \n",&next));
        cnt = atoi(strtok_r(NULL," \n",&next));
        cnt *= -1;
        int state = update_stock(id, cnt);
        switch (state){
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
    }
    else if (strcmp(prefix, "sell") == 0)
    {
        int id, cnt;
        id = atoi(strtok_r(NULL, " \n", &next));
        cnt = atoi(strtok_r(NULL, " \n", &next));
     
        int state = update_stock(id, cnt);
        switch (state)
        {
        case SUCCESS:
            strcpy(result,"[sell] \033[0;32msuccess\033[0m\n");
            break;
        
        case NOT_FOUND:
            strcpy(result,"Stock Not found\n");
            break;
        }
    }
    // V(&buf_mutex);
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
    //printf("%s\n", stock_info);
    fclose(fp);
    return 1;
}

