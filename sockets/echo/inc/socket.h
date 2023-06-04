#ifndef SOCK_H
#define SOCK_H

#define ECHO_SERVER_NAME "/tmp/echo_server"
#define ECHO_LOC_CLIENT "/tmp/echo_client"
#define ECHO_SERVER_IP "127.0.0.1"
#define ECHO_SERVER_PORT 3333
#define EXIT_WORD "exit"

#define MSG_DATA_MAX_LEN   500

int get_user_input_string(char *str, int max_string_size);

#endif /*SOCK_H*/