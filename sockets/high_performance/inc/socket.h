#ifndef SOCK_H
#define SOCK_H

#define ECHO_SERVER_NAME "/tmp/echo_server"
#define ECHO_LOC_CLIENT "/tmp/echo_client"
#define ECHO_SERVER_IP "127.0.0.1"
#define ECHO_SERVER_PORT 3333
#define ECHO_SERVER_DGRAM_PORT 3334
#define EXIT_WORD "exit"

#define START_WORK_THREADS_NUM 2
#define USER_QUEUE_LEN 100

#define MSG_DATA_MAX_LEN   500

typedef struct {
    void *previous_user;
    void *next_user;
    int user_fd;
    pthread_t user_thread;
} user_list;

typedef struct {
    int read_index;
    int write_index;
    int user_fd[USER_QUEUE_LEN];
} user_queue;


int get_user_input_string(char *str, int max_string_size);
user_list *del_user_from_list(user_list *user);

#endif /*SOCK_H*/