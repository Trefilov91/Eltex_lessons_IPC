#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "socket.h"

int tcp_fd, udp_fd, user_fd, epoll_fd;

void *thread_echo_server(void *args)
{
    char msg_str[MSG_DATA_MAX_LEN];
    struct epoll_event ctl_event, wait_event;
    socklen_t client_addr_size;  
    struct sockaddr_in client;
    int ret;

    epoll_fd = epoll_create(3);
    if(epoll_fd == -1) {
        perror("epoll_create");
        close(tcp_fd);
        close(udp_fd);
        exit(EXIT_FAILURE);   
    }

    ctl_event.events = EPOLLIN;
	ctl_event.data.fd = tcp_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_fd, &ctl_event) == -1) {
        perror("epoll_ctl: listen_sock");
        close(tcp_fd);
        close(udp_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE); 
    }

    ctl_event.events = EPOLLIN;
	ctl_event.data.fd = udp_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_fd, &ctl_event) == -1) {
        perror("epoll_ctl: listen_sock");
        close(tcp_fd);
        close(udp_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE); 
    }

    while(1) {

        ret = epoll_wait(epoll_fd, &wait_event, 1, -1);    
        if(ret == -1) {
            perror("epoll_wait");
            close(tcp_fd);
            close(udp_fd);
            close(epoll_fd);       
            exit(EXIT_FAILURE);        
        }   

        if (wait_event.data.fd == tcp_fd) {
            client_addr_size = sizeof(client);
            user_fd = accept(tcp_fd, (struct sockaddr *)&client, &client_addr_size);
            if(user_fd == -1) {
                perror("accept");
                close(tcp_fd);
                close(udp_fd);
                close(epoll_fd); 
                exit(EXIT_FAILURE);        
            } 
            ctl_event.events = EPOLLIN;
	        ctl_event.data.fd = user_fd;
            if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, user_fd, &ctl_event) == -1) {
                perror("epoll_ctl: listen_sock");
                close(tcp_fd);
                close(udp_fd);
                close(epoll_fd);
                exit(EXIT_FAILURE); 
            }        
        }
        else if (wait_event.data.fd == user_fd) {
            memset(msg_str, 0, sizeof(msg_str));
            ret = recv(user_fd, msg_str, sizeof(msg_str), 0); 
            if(ret == -1) {
                perror("recv");
                close(tcp_fd);
                close(udp_fd);
                close(epoll_fd);  
                close(user_fd);   
                exit(EXIT_FAILURE);        
            }            
            else if(!ret) {
                ctl_event.events = EPOLLIN;
	            ctl_event.data.fd = user_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, user_fd, &ctl_event);
                close(user_fd);            
            }
            else {
                strcat(msg_str, " (msg received)\n");
                ret = send(user_fd, msg_str, strlen(msg_str), 0); 
                if(ret == -1) {
                    perror("send");
                    close(tcp_fd);
                    close(udp_fd);
                    close(epoll_fd);  
                    close(user_fd);    
                    exit(EXIT_FAILURE);        
                } 
            }
        }
        else if (wait_event.data.fd == udp_fd) {
            memset(msg_str, 0, sizeof(msg_str));
            client_addr_size = sizeof(client);
            ret = recvfrom(udp_fd, msg_str, sizeof(msg_str), 0, (struct sockaddr *)&client, &client_addr_size); 
            if(ret == -1) {
                perror("recvfrom");
                close(tcp_fd);
                close(udp_fd);
                close(epoll_fd);  
                close(user_fd);    
                exit(EXIT_FAILURE);        
            } 
            else if(!ret) 
                continue;

            strcat(msg_str, " (msg received)\n");
            ret = sendto(udp_fd, msg_str, strlen(msg_str), 0, (struct sockaddr *)&client, client_addr_size); 
            if(ret == -1) {
                perror("sendto");
                close(tcp_fd);
                close(udp_fd);
                close(epoll_fd);  
                close(user_fd);         
                exit(EXIT_FAILURE);        
            }            
        } 
    }
    return 0;
}

int main()
{
    struct sockaddr_in server;
    int ret;      
    pthread_t thread_echo;
    int *thread_status;
    char input_buf[100];

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);        
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(ECHO_SERVER_IP);

    ret = bind(tcp_fd, (struct sockaddr *)&server, sizeof(server));
    if(ret == -1) {
        perror("bind");
        close(tcp_fd);
        exit(EXIT_FAILURE);          
    }
    ret = listen(tcp_fd, 5);
    if(ret == -1) {
        perror("listen");
        close(tcp_fd);
        exit(EXIT_FAILURE);          
    }  

    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd == -1) {
        perror("socket");
        close(tcp_fd);
        exit(EXIT_FAILURE);        
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_SERVER_DGRAM_PORT);
    server.sin_addr.s_addr = inet_addr(ECHO_SERVER_IP);
    
    ret = bind(udp_fd, (struct sockaddr *)&server, sizeof(server));
    if(ret == -1) {
        perror("bind");
        close(tcp_fd);
        close(udp_fd);
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
    close(user_fd);
    close(tcp_fd);
    close(udp_fd);
    close(epoll_fd);
    return 0;
}


