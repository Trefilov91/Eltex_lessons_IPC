#ifndef MSGQ_H
#define MSGQ_H

#define SERWER_QUEUE "/queue_to_server"
#define EXIT_WORD "exit"

#define DEFAULT_PRIORITY 5
#define USER_NAME_MAX_LEN  50
#define MSG_DATA_MAX_LEN   500

typedef struct {
    char user_name[USER_NAME_MAX_LEN];
    char data[MSG_DATA_MAX_LEN];
} msg_t;

int get_user_input_string(char *str, int max_string_size);

#endif /*MSGQ_H*/