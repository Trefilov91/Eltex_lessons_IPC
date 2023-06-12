#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "socket.h"

int passiv_fd;
user_list *last_user = NULL;
user_list *current_user= NULL;

void *thread_echo_work(void *args)
{
    int user_fd;
    char msg_str[MSG_DATA_MAX_LEN];
    int ret;

    printf("thread_echo_work start\n");

    while(1) {
        while(1) {
            user_fd = *(int *)args;
            if(user_fd >= 0)
                break;
        }

        while(1) {
            memset(msg_str, 0, sizeof(msg_str));
            ret = recv(user_fd, msg_str, sizeof(msg_str), 0); 
            if(ret == -1) {
                perror("recv");
                close(user_fd);      
                exit(EXIT_FAILURE);        
            }    
            else if(!ret) {
                close(user_fd);
                *(int *)args = -1;
                break;
            }
            strcat(msg_str, " (msg received)\n");
            ret = send(user_fd, msg_str, strlen(msg_str), 0); 
            if(ret == -1) {
                perror("send");
                close(user_fd);     
                exit(EXIT_FAILURE);        
            }  
        }     
    }
}

void *thread_echo_listener(void *args)
{
    socklen_t client_addr_size;  
    struct sockaddr_in client;
    int *thread_status;
    int ret, i;

    for(i = 0; i < START_WORK_THREADS_NUM; i++) {
        current_user = last_user;
        last_user = malloc(sizeof(user_list));
        last_user->previous_user = current_user;
        last_user->next_user = NULL;
        if(current_user)
            current_user->next_user = last_user;
        last_user->user_fd = -1;
        pthread_create(&last_user->user_thread, NULL, thread_echo_work, &last_user->user_fd);      
    }

    while(1) {
        current_user = last_user;
        while(current_user) {
            printf("current_user=%p, next=%p, prev=%p\n", current_user, current_user->next_user, current_user->previous_user);
            if(current_user->user_fd < 0)
                break;
            else
                current_user = current_user->previous_user;
        }

        if(current_user == NULL) {
            current_user = last_user;
            last_user = malloc(sizeof(user_list));
            last_user->previous_user = current_user;
            last_user->next_user = NULL;
            if(current_user)
                current_user->next_user = last_user;
            last_user->user_fd = -1;
            pthread_create(&last_user->user_thread, NULL, thread_echo_work, &last_user->user_fd);
            current_user = last_user;          
        }

        client_addr_size = sizeof(client);
        current_user->user_fd = accept(passiv_fd, (struct sockaddr *)&client, &client_addr_size);
        if(current_user->user_fd == -1) {
            perror("accept");
            close(passiv_fd);
            exit(EXIT_FAILURE);        
        }   

        current_user = last_user;
        i = 0;
        while(current_user) {
            printf("current_user=%p, user_fd=%d\n", current_user, current_user->user_fd);
            if(current_user->user_fd < 0) {                
                if(++i > START_WORK_THREADS_NUM-1) {
                    pthread_cancel(current_user->user_thread);
                    del_user_from_list(current_user);  
                    printf("thread canceled\n");
                    break;
                }
            }
            current_user = current_user->previous_user;
        } 

    }
    return 0;
}

int main()
{
    struct sockaddr_in server;
    int ret;      
    pthread_t echo_listener;
    int *thread_status;
    char input_buf[100];

    passiv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(passiv_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);        
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(ECHO_SERVER_IP);

    ret = bind(passiv_fd, (struct sockaddr *)&server, sizeof(server));
    if(ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);          
    }

    ret = listen(passiv_fd, 5);
    if(ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);          
    }    
    pthread_create(&echo_listener, NULL, thread_echo_listener, NULL);  
  

    printf("введите '%s' для выхода\n", EXIT_WORD);
    while(1) {
        memset(input_buf, 0, sizeof(input_buf));
        get_user_input_string(input_buf, sizeof(input_buf));  

        if(!strcmp(input_buf, EXIT_WORD)) {
            break;
        }          
    }

    pthread_cancel(echo_listener);
    close(passiv_fd);

    while(last_user) {
        if(last_user->user_thread)
            pthread_cancel(last_user->user_thread);
        
        last_user = del_user_from_list(last_user);    
    }

    return 0;
}


