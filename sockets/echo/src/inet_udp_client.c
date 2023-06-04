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

int fd;

void *thread_msg_receive(void *args)
{
    char msg_str[MSG_DATA_MAX_LEN];
    int ret;

    while(1) {
        memset(msg_str, 0, sizeof(msg_str));
        ret = recv(fd, msg_str, sizeof(msg_str), 0); 
        if(ret == -1) {
            perror("recv");
            close(fd);            
            exit(EXIT_FAILURE);        
        }    

        printf("%s", msg_str);
    }
    return NULL;
}

int main()
{
    struct sockaddr_in server;
    int ret;      
    pthread_t thread_receive;
    int *thread_status;
    char input_buf[MSG_DATA_MAX_LEN];

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);        
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(ECHO_SERVER_IP);
    
    ret = connect(fd, (struct sockaddr *)&server, sizeof(server));
    if(ret == -1) {
        perror("connect");
        close(fd);
        exit(EXIT_FAILURE);          
    }

    pthread_create(&thread_receive, NULL, thread_msg_receive, NULL);  
  
    printf("введите '%s' для выхода\n", EXIT_WORD);
    while(1) {
        memset(input_buf, 0, sizeof(input_buf));
        get_user_input_string(input_buf, sizeof(input_buf));  

        if(!strcmp(input_buf, EXIT_WORD))
            break;

        ret = send(fd, input_buf, strlen(input_buf), 0); 
        if(ret == -1) {
            perror("send");
            break;
        }         
    }

    pthread_cancel(thread_receive);
    close(fd);

    return 0;
}


