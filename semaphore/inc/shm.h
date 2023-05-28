#ifndef SHM_H
#define SHM_H

#define CHAT_SHM_NAME "/chat_shared_mem"
#define CHAT_SEMAPHORE_NAME "/chat_semaphore"
#define EXIT_WORD "exit"

#define MSG_DATA_MAX_NUM   20
#define USER_NAME_MAX_LEN  50
#define MSG_DATA_MAX_LEN   500

typedef struct {
    char user_name[USER_NAME_MAX_LEN];
    char data[MSG_DATA_MAX_LEN];
} chat_msg_t;

typedef struct {
    int next_msg_index;
    chat_msg_t msg[MSG_DATA_MAX_NUM];
} chat_shm_t;

int get_user_input_string(char *str, int max_string_size);

#endif /*SHM_H*/