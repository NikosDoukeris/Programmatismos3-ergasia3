#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "ergasialib.h"

#define LIST_LEN 20
#define ORDERS_PER_CUSTOMER 10
#define SOCKETNAME "eshop"

struct product{
    char description[256];
    int price;
    int item_count;
    int item_req;
    int sold;
    int unserved;
};

struct results_s{
    int failed_sum;
    int done_sum;
    int sum;
    int money_sum;
};

struct product products[LIST_LEN];
struct results_s results;

void displayResults(){
    blue();
    printf("\n                             ***********");
    printf("\n                             * RESULTS *");
    printf("\n                             ***********\n");
    clear();

    for(int p=0; p<LIST_LEN; p++){
        printf("%s,  ", products[p].description);
        printf("Price: %d,  ", products[p].price);
        printf("Count: %d,  ", products[p].item_count);
        printf("Requests: %d,  ", products[p].item_req);
        printf("Sold: %d,  ", products[p].sold);
        printf("Unserved: %d\n", products[p].unserved);
    }

    results.sum = results.done_sum + results.failed_sum;
    printf("==================================================================\n");
    printf("Orders:%d, Successful orders:%d, Failed orders:%d, Total spent:%d\n", results.sum, results.done_sum, results.failed_sum, results.money_sum);
    printf("==================================================================\n");
}

void createProducts(){
    for(int i=0; i<LIST_LEN; i++){
        // int -> string
        char temp_str[16];
        sprintf(temp_str, "Product %d", i+1);

        strcpy(products[i].description, temp_str);
        products[i].price = randomInt(50);
        products[i].item_count = 2;
        products[i].item_req = 0;
        products[i].sold = 0;
        products[i].unserved = 0;

        cyan();
        printf("%s\n", products[i].description);
        clear();
        printf("Price: %d\n", products[i].price);
        printf("Count: %d\n------------\n", products[i].item_count);
        sleep(1);
    }
    printf("================================\n");
}

int main(void) {

    createProducts();

    for(int i=0; i<5; i++){
    
        struct sockaddr_un sa;
        sa.sun_family = AF_UNIX;
        (void)unlink(SOCKETNAME);
        strcpy(sa.sun_path, SOCKETNAME);
        int fd_skt, fd_client; char buf[100]; 
        int written; ssize_t readb;

        yellow();
        printf("\nCustomer %d:\n", i+1);
        clear();

        if (fork() == 0) { /* client */
            //START connection
            if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
                red();
                perror("Message from socket [client] : ");
                clear();
                exit(-1);
            }
            while (connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
                if (errno == ENOENT) {
                    sleep(1);
                    continue;
                }
                else {
                    red();
                    perror("[Client] Message from connect");
                    clear();
                    printf("[Client] Trying again...\n");
                }
            }
            //END connection

            int pid = getpid();
            written = write(fd_skt, &pid, sizeof(int));
            if (written==-1) {
                red();
                perror("Message from write [client]");
                clear();
                exit(-2);
            }
            //else printf ("[Client %d] %d bytes written to server\n", getpid(), written);
                        
            readb = read(fd_skt, buf, sizeof(buf));
            if (readb==-1) {
                red();
                perror("Message from read [client]");
                clear();
                exit(-3); 
            } 
            //else printf ("[Client %d] %zd bytes read from server\n", getpid(), readb); 
            printf("\n");


            //START client func
            for(int order_c=0; order_c<ORDERS_PER_CUSTOMER; order_c++){
                sleep(1);
                int product_index = randomInt(LIST_LEN);
                write(fd_skt, &product_index, sizeof(int));
                int ans;
                read(fd_skt, &ans, sizeof(int));
                if(ans == 1) printf("[Client] msg from server: Order successful\n\n");
                else printf("[Client] msg from server: Order failed\n\n");
                
                //xoris ta kato oi times sta struct ginontai 0
                int temp_1 = 0;
                write(fd_skt, &temp_1, sizeof(int));
                read(fd_skt, &products, sizeof(products));
                write(fd_skt, &temp_1, sizeof(int));
                read(fd_skt, &results, sizeof(results));
            }    
            //END client func

            close(fd_skt);
            //exit(0);
        }    
        else { /* server */
            //START connection
            if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
                red();
                perror("Message from socket [server]");
                clear();
                exit(-1);
            }        
            if (bind(fd_skt, (struct sockaddr *) &sa, sizeof(sa))){
                red();
                perror("Message from bind [server]");
                clear();
                exit(-2);
            }        
            if (listen(fd_skt, 1)){//5
                red();
                perror("Message from listen [server]");
                clear();
                exit(-3); 
            }
            if ((fd_client = accept(fd_skt, NULL, 0))<0) {
                red();
                perror("Message from accept [server]"); 
                clear();
                exit(-4); 
            }
            //END connection

            int sent_pid;
            readb = read(fd_client, &sent_pid, sizeof(int));
            if (readb == -1) {
                perror("Message from read [server] : ");
                exit(-5);
            }
            if (write(fd_client, "a", 2)==-1) {
                perror("Message from write [server]");
                exit(-6);
            }



            //START server func
            for(int order_p=0; order_p<ORDERS_PER_CUSTOMER; order_p++){
                int product_num;
                read(fd_client, &product_num, sizeof(int));
                printf("[Server] Product id: %d\n", product_num);

                int temp_sum = 0;
                int temp_ans;
                products[product_num].item_req += 1;
                if(products[product_num].item_count > 0){ //successfull order
                    //edit struct
                    products[product_num].item_count--;
                    products[product_num].sold++;
                    temp_sum += products[product_num].price;

                    //sum
                    results.done_sum++;
                    results.money_sum += products[product_num].price;

                    temp_ans = 1;
                }else{  //failed order
                    products[product_num].unserved++;

                    //sum
                    results.failed_sum++;

                    temp_ans = 0;
                }
                write(fd_client, &temp_ans, sizeof(int));
                results.sum++;

                //xoris ta kato oi times sta struct ginontai 0
                int temp_2;
                read(fd_client, &temp_2, sizeof(int));
                write(fd_client, &products, sizeof(products));
                read(fd_client, &temp_2, sizeof(int));
                write(fd_client, &results, sizeof(results));
            }
            //END server func

            close(fd_skt);
            close(fd_client);
            wait(NULL);
            exit(0);
        }
        printf("================================\n");
        usleep(500 * 1000);
    }

    displayResults();
    return 0;
}


