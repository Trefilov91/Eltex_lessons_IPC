#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <pthread.h>
#include "socket.h"

const uint8_t echo_server_mac[ETH_ALEN] = {0x08,0x00,0x27,0xf0,0xc3,0x28};
const uint8_t echo_client_mac[ETH_ALEN] = {0x08,0x00,0x27,0x54,0x01,0xd6};

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
    struct ether_header eth_header;
    ip_header_t  ip_header;
    udp_header_t udp_header;
} packet_header_t;
#pragma pack(pop)

int fd;

uint16_t calc_checksum(uint16_t *buf, int len)
{
    uint32_t sum;
    for(sum=0; len > 0; len--) {
        sum += *buf++;
    }
    sum = (sum >> 16) + (sum &0xffff);
    return (uint16_t)(~sum);
}

void *thread_msg_receive(void *args)
{
    int ret;
    struct sockaddr_ll server;
    socklen_t server_addr_size;

    char rx_buf[MSG_DATA_MAX_LEN];
    udp_header_t *rx_udp_header = (udp_header_t *)(rx_buf + sizeof(struct ether_header) + sizeof(ip_header_t));
    char *rx_udp_payload = (char *)(rx_buf + sizeof(packet_header_t));

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
    }
    return NULL;
}

int main()
{
    pthread_t thread_receive;
    int ret, optval = 1;
    struct sockaddr_ll server;
    
    char tx_buf[MSG_DATA_MAX_LEN];    
    struct ether_header *tx_eth_header = (struct ether_header *)tx_buf;
    ip_header_t * tx_ip_header = (ip_header_t *)(tx_buf + sizeof(struct ether_header));
    udp_header_t *tx_udp_header = (udp_header_t *)(tx_buf + sizeof(struct ether_header) + sizeof(ip_header_t));
    char *input_buf = (char *)(tx_buf + sizeof(packet_header_t));
    
    memset(&tx_buf, 0, sizeof(packet_header_t));

    memcpy(tx_eth_header->ether_dhost, echo_server_mac, ETH_ALEN);
    memcpy(tx_eth_header->ether_shost, echo_client_mac, ETH_ALEN);
    tx_eth_header->ether_type = htons(NET_PROTOCOL_IPv4);

    tx_ip_header->version = IP_VERSION_4;
    tx_ip_header->ihl     = DEFAULT_IHL;
    tx_ip_header->flags_offset = IP_FLAG_DF;
    tx_ip_header->ttl     = 255;
    tx_ip_header->protocol = IPPROTO_UDP;
    tx_ip_header->destination_address = inet_addr(ECHO_SERVER_IP);
    tx_ip_header->source_address = inet_addr(ECHO_CLIENT_IP);
    tx_ip_header->packet_id = htons(1);  

    tx_udp_header->source_port = htons(ECHO_CLIENT_PORT);
    tx_udp_header->destination_port = htons(ECHO_SERVER_PORT);    
  
    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);        
    }

    memset(&server, 0, sizeof(server));
    server.sll_family = AF_PACKET;
    server.sll_ifindex = if_nametoindex(CLIENT_INTERFACE_NAME);
    server.sll_halen = ETH_ALEN;
    memcpy(server.sll_addr, echo_server_mac, ETH_ALEN);   

    pthread_create(&thread_receive, NULL, thread_msg_receive, NULL);  

    printf("введите '%s' для выхода\n", EXIT_WORD);
    while(1) {
        memset(input_buf, 0, sizeof(tx_buf) - sizeof(packet_header_t));
        get_user_input_string(input_buf, sizeof(tx_buf) - sizeof(packet_header_t));  

        if(!strcmp(input_buf, EXIT_WORD))
            break;

        tx_udp_header->lenght = htons(sizeof(udp_header_t) + strlen(input_buf));
        tx_ip_header->lenght = htons(sizeof(ip_header_t) + sizeof(udp_header_t) + strlen(input_buf));
        tx_ip_header->checksum = 0;
        tx_ip_header->checksum = calc_checksum((uint16_t *)tx_ip_header, sizeof(ip_header_t) / sizeof(uint16_t));

        ret = sendto(fd, tx_buf, sizeof(packet_header_t) + strlen(input_buf), 0, (struct sockaddr *)&server, sizeof(server)); 
        if(ret == -1) {
            perror("sendto");
            pthread_cancel(thread_receive);            
            close(fd);            
            exit(EXIT_FAILURE);
        }  
    }

    pthread_cancel(thread_receive);
    close(fd);

    return 0;
}

