#ifndef SOCK_H
#define SOCK_H

#define ECHO_SERVER_NAME "/tmp/echo_server"
#define ECHO_LOC_CLIENT "/tmp/echo_client"
#define ECHO_SERVER_IP "127.0.0.1"
#define ECHO_SERVER_PORT 3333
#define ECHO_CLIENT_PORT 58153
#define EXIT_WORD "exit"

#define IP_VERSION_4       4
#define DEFAULT_IHL        5

#define IP_FLAGS_OFSET     5
#define IP_FLAG_DF         (0x2 << IP_FLAGS_OFSET)
#define IP_FLAG_MF         (0x1 << IP_FLAGS_OFSET)

#define MSG_DATA_MAX_LEN   500
#define IP_HEADER_LEN      20



int get_user_input_string(char *str, int max_string_size);

#endif /*SOCK_H*/