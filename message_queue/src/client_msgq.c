#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <errno.h>
#include "msg_queue.h"

int msg_len;
mqd_t fd_write;
msg_t msg;
unsigned char exit_flag = 0;

void *thread_user_input(void *args)
{
    char input_buf[MSG_DATA_MAX_LEN];
    printf("введите '%s' для выхода\n", EXIT_WORD);
    while(1) {
        memset(input_buf, 0, sizeof(input_buf));
        memset(msg.data, 0, sizeof(msg.data));
        get_user_input_string(input_buf, sizeof(input_buf));        
        strcpy(msg.data, input_buf);
        msg_len = sizeof(msg.user_name) + strlen(msg.data);
        if(mq_send(fd_write,(char *)&msg, msg_len, DEFAULT_PRIORITY) == -1) {
            if(errno != EAGAIN)
                break;
        }
        if(!strcmp(input_buf, EXIT_WORD))             
            break;
    }
    exit_flag = 1;
    return NULL;
}

int main(int argc, char **argv)
{
    char msg_str[USER_NAME_MAX_LEN + MSG_DATA_MAX_LEN + 2];
    char msgq_name[100];
    int readed_bytes;
    int offset;
    mqd_t fd_read; 
    struct mq_attr attr;    
    pid_t own_pid = getpid();
    pthread_t thread_input;
    int *thread_status;

    if(argc != 3)
        printf("Print your name and sername as atribut\n");

    memset(msg.user_name, '0', sizeof(msg.user_name));
    memset(msg.data, 0, sizeof(msg.data));
    offset = sprintf(msg.user_name, "%s %s", argv[1], argv[2]);
    msg.user_name[offset] = ':';

    memset(msgq_name, 0, sizeof(msgq_name));
    msgq_name[0] = '/';
    sprintf(msgq_name, "/%ld", (long)own_pid);
    strcpy(msg.data, msgq_name);

    fd_read = mq_open(msgq_name, O_CREAT | O_EXCL | O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL);
    if(fd_read < 0) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    mq_getattr(fd_read, &attr);

    do {
        fd_write = mq_open(SERWER_QUEUE, O_WRONLY | O_NONBLOCK);
    } while(fd_write < 0);

    msg_len = sizeof(msg.user_name) + strlen(msg.data);
    mq_send(fd_write,(char *)&msg, msg_len, DEFAULT_PRIORITY);

    pthread_create(&thread_input, NULL, thread_user_input, NULL);

    memset(msg_str, 0, sizeof(msg_str));

    while(!exit_flag) {
        readed_bytes = mq_receive(fd_read, msg_str, attr.mq_msgsize, 0);
        if(readed_bytes != -1) {
            msg_str[readed_bytes] = '\n';
            msg_str[readed_bytes+1] = '\0';
            printf("%s", msg_str);
            fflush(stdout);
            memset(msg_str, 0, sizeof(msg_str));  
        }
        else if(errno != EAGAIN) {
            perror("mq_receive() ");
            break; 
        }     
    }

    pthread_join(thread_input, (void **)&thread_status);
    close(fd_write);
    close(fd_read); 

    mq_unlink(msgq_name);

    exit(EXIT_SUCCESS);
}

