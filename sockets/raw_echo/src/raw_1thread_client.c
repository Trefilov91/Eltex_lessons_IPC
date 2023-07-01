#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include "socket.h"

#pragma pack(push,1)
typedef struct {
    uint8_t  ihl     :4;
    uint8_t  version :4;    
    uint8_t  ds;
    uint16_t lenght;
    uint16_t packet_id; 
    uint16_t flags_offset;      
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t source_address;
    uint32_t destination_address;
} ip_header_t;

typedef struct {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t lenght;
    uint16_t checksum;
} udp_header_t;

typedef struct {
    ip_header_t  ip_header;
    udp_header_t udp_header;
} packet_header_t;
#pragma pack(pop)

int main()
{
    int fd, ret, optval = 1;
    struct sockaddr_in server;
    socklen_t server_addr_size;

    char tx_buf[MSG_DATA_MAX_LEN];
    char rx_buf[MSG_DATA_MAX_LEN];
    ip_header_t * tx_ip_header = (ip_header_t *)tx_buf;
    udp_header_t *tx_udp_header = (udp_header_t *)(tx_buf + sizeof(ip_header_t));
    char *input_buf = (char *)(tx_buf + sizeof(packet_header_t));
    
    udp_header_t *rx_udp_header = (udp_header_t *)(rx_buf + sizeof(ip_header_t));
    char *rx_udp_payload = (char *)(rx_buf + sizeof(packet_header_t));

    memset(&tx_buf, 0, sizeof(packet_header_t));

    
    tx_ip_header->version = IP_VERSION_4;
    tx_ip_header->ihl     = DEFAULT_IHL;
    tx_ip_header->flags_offset = IP_FLAG_DF;
    tx_ip_header->ttl     = 255;
    tx_ip_header->protocol = IPPROTO_UDP;
    tx_ip_header->destination_address = inet_addr(ECHO_SERVER_IP);  

    tx_udp_header->source_port = htons(ECHO_CLIENT_PORT);
    tx_udp_header->destination_port = htons(ECHO_SERVER_PORT);    
  
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);        
    }

    if(setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(fd); 
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(ECHO_SERVER_IP);    

    printf("введите '%s' для выхода\n", EXIT_WORD);
    while(1) {
        memset(input_buf, 0, sizeof(tx_buf) - sizeof(packet_header_t));
        get_user_input_string(input_buf, sizeof(tx_buf) - sizeof(packet_header_t));  

        if(!strcmp(input_buf, EXIT_WORD))
            break;

        tx_udp_header->lenght = htons(sizeof(udp_header_t) + strlen(input_buf));
       
        ret = sendto(fd, tx_buf, sizeof(packet_header_t) + strlen(input_buf), 0, (struct sockaddr *)&server, sizeof(server)); 
        if(ret == -1) {
            perror("sendto");            
            close(fd);            
            exit(EXIT_FAILURE);
        }  

        while(1) {
            memset(rx_buf, 0, sizeof(rx_buf));
            server_addr_size = sizeof(server);
            ret = recvfrom(fd, rx_buf, sizeof(rx_buf), 0, (struct sockaddr *)&server, &server_addr_size);
            if(ret == -1) {
                perror("recvfrom");
                close(fd);            
                exit(EXIT_FAILURE);        
            }
            
            if(ntohs(rx_udp_header->destination_port) != ECHO_CLIENT_PORT)
                continue;

            printf("%s", rx_udp_payload);
            break;
        }
    }

    close(fd);

    return 0;
}

