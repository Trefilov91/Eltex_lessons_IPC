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
struct sockaddr_un own_addr;

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
            unlink(own_addr.sun_path);      
            exit(EXIT_FAILURE);        
        }    

        printf("%s", msg_str);
    }
    return NULL;
}

int main()
{
    struct sockaddr_un server;
    int ret, pid = getpid();      
    pthread_t thread_receive;
    int *thread_status;
    char input_buf[MSG_DATA_MAX_LEN];

    fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);        
    }
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    strcpy(server.sun_path, ECHO_SERVER_NAME);
    
    memset(&own_addr, 0, sizeof(own_addr));
    own_addr.sun_family = AF_LOCAL;
    sprintf(own_addr.sun_path, "%s_%d", ECHO_LOC_CLIENT, pid);

    ret = bind(fd, (struct sockaddr *)&own_addr, sizeof(own_addr));
    if(ret == -1) {
        perror("bind");
        close(fd);
        unlink(own_addr.sun_path);
        exit(EXIT_FAILURE);          
    }

    ret = connect(fd, (struct sockaddr *)&server, sizeof(server));
    if(ret == -1) {
        perror("connect");
        close(fd);
        unlink(own_addr.sun_path);
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
    unlink(own_addr.sun_path);

    return 0;
}


