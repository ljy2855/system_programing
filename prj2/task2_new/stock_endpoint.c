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
    int read_cnt;
    sem_t mutex, mutex_read;
};

Stock *root_tree = NULL;
/**
 * @brief insert new stock in bst
 * 
 * @param ID 
 * @param price 
 * @param cnt 
 */
void insert_stock(int ID, int price, int cnt){
    Stock *new_stock = (Stock *)malloc(sizeof(Stock));
    Sem_init(&new_stock->mutex,0,1);
    Sem_init(&new_stock->mutex_read,0,1);
    new_stock->ID = ID;
    new_stock->price = price;
    new_stock->cnt = cnt;
    new_stock->left_stock = NULL;
    new_stock->right_stock = NULL;
    
    if (root_tree == NULL)
    {
        // if tree is empty
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
/**
 * @brief Get the stock object from bst
 * 
 * @param ID 
 * @return Stock* 
 */
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


/**
 * @brief Set the stocks info object to stock_info
 * 
 * @param cur 
 * @param stock_info 
 */
void set_stocks_info(Stock *cur,char stock_info[])
{
  
    
    char temp[200];
    if(cur != NULL){
        P(&cur->mutex_read);
        cur->read_cnt++;
        if (cur->read_cnt == 1) /* First in */
            P(&cur->mutex);
        V(&cur->mutex_read);

        //critical section to read
        sprintf(temp, "%d %d %d\n",cur->ID,cur->cnt,cur->price);
        strcat(stock_info, temp);

        P(&cur->mutex_read);
        cur->read_cnt--;
        if (cur->read_cnt == 0) /* Last out */
            V(&cur->mutex);
        V(&cur->mutex_read);

        set_stocks_info(cur->left_stock,stock_info);
        set_stocks_info(cur->right_stock,stock_info);
        
    }
    
}
/**
 * @brief write stock data from memory to file
 * 
 */
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

/**
 * @brief update cnt target stock
 * 
 * @param ID 
 * @param cnt 
 * @return int 
 */
int update_stock(int ID, int cnt)
{
    Stock *target_stock = get_stock(ID);
    P(&target_stock->mutex); // write lock
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

/**
 * @brief process client request and return message
 * 
 * @param command 
 * @return char* 
 */
char * process_request(char command[]){
    
    char *prefix;
    char *next;
    char *result = (char *)malloc(sizeof(char) * MAXBUF);

    memset(result,0,MAXBUF);
    prefix = strtok_r(command, " \n",&next);
    if(strcmp(prefix,"show")== 0){
        // if request is show
        set_stocks_info(root_tree, result);
        
    }
    else if (strcmp(prefix, "buy") == 0)
    {
        //if request is buy
        int id, cnt;

        //parse request
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
        //if request is sell
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
/**
 * @brief control-c handler that write file before exit
 * 
 * @param sig 
 */
void terminate_handler(int sig){
    printf("write to file...\n");
    write_to_file();
    printf("write file success\n");
    printf("server close\n");
    exit(0);
}

/**
 * @brief intialize bst from stock.txt
 * 
 * @return int 
 */
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

