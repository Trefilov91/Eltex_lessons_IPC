#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "socket.h"

int fd;

void *thread_echo_server(void *args)
{
    char msg_str[MSG_DATA_MAX_LEN];
    socklen_t client_addr_size;  
    struct sockaddr_un client;
    int ret;    
    while(1) {
        memset(msg_str, 0, sizeof(msg_str));
        memset(&client, 0, sizeof(client));
        client_addr_size = sizeof(client);
        ret = recvfrom(fd, msg_str, sizeof(msg_str), 0, (struct sockaddr *)&client, &client_addr_size); 
        if(ret == -1) {
            perror("recvfrom");
            close(fd); 
            unlink(ECHO_SERVER_NAME);           
            exit(EXIT_FAILURE);        
        }    
        else if(!ret) 
            continue;

        strcat(msg_str, " (msg received)\n");
        ret = sendto(fd, msg_str, strlen(msg_str), 0, (struct sockaddr *)&client, client_addr_size); 
        if(ret == -1) {
            perror("sendto");
            close(fd);  
            unlink(ECHO_SERVER_NAME);          
            exit(EXIT_FAILURE);        
        }  
    }
    return 0;
}

int main()
{
    struct sockaddr_un server;
    int ret;      
    pthread_t thread_echo;
    int *thread_status;
    char input_buf[100];

    fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);        
    }
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_LOCAL;    
    strcpy(server.sun_path, ECHO_SERVER_NAME);
    
    ret = bind(fd, (struct sockaddr *)&server, sizeof(server));
    if(ret == -1) {
        perror("bind");
        close(fd);
        unlink(ECHO_SERVER_NAME);
        exit(EXIT_FAILURE);          
    }
 
    pthread_create(&thread_echo, NULL, thread_echo_server, NULL);  
  
    printf("введите '%s' для выхода\n", EXIT_WORD);
    while(1) {
        memset(input_buf, 0, sizeof(input_buf));
        get_user_input_string(input_buf, sizeof(input_buf));  

        if(!strcmp(input_buf, EXIT_WORD)) {
            break;
        }          
    }

    pthread_cancel(thread_echo);
    close(fd);
    unlink(ECHO_SERVER_NAME);
    return 0;
}


