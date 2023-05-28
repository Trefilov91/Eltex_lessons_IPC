#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <malloc.h>
#include <pthread.h>
#include <errno.h>
#include "msg_queue.h"

typedef struct {
    void *next_user;
    void *previous_user;
    char user_name[50];
    char user_msgq[50];
    mqd_t user_fd;
} user_list;

void del_user_from_list(user_list *user);
unsigned char exit_flag = 0;

void *thread_user_input(void *args)
{
    char input_buf[10];
    printf("введите 'exit' для выхода\n");
    while(1) {
        get_user_input_string(input_buf, sizeof(input_buf));
        if(!strcmp(input_buf, "exit")) {
            exit_flag = 1;       
            break;
        }
    }
    return NULL;
}

int main()
{

    char msg_buf[USER_NAME_MAX_LEN + MSG_DATA_MAX_LEN + 2];
    int offset = 0;
    mqd_t fd_read; 
    struct mq_attr attr;
    user_list *last_user = NULL;
    user_list *current_user= NULL;
    msg_t msg;
    pthread_t thread_input;
    int *thread_status;

    mq_unlink(SERWER_QUEUE);  

    do {
        fd_read = mq_open(SERWER_QUEUE, O_CREAT | O_EXCL | O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL);
        if(fd_read < 0) {
            perror("mq_open");
            exit(EXIT_FAILURE);
        }
    } while(fd_read < 0);
    mq_getattr(fd_read, &attr);

    pthread_create(&thread_input, NULL, thread_user_input, NULL);

    while(1) {   
        memset((char *)&msg, 0, sizeof(msg));
        if(mq_receive(fd_read, (char *)&msg, attr.mq_msgsize, 0) != -1) {
            for(current_user = last_user; current_user != NULL; 
                current_user = current_user->previous_user) {             
                if( !strncmp(current_user->user_name, msg.user_name, USER_NAME_MAX_LEN) )
                    break;
            }

            if(current_user == NULL) {
                current_user = last_user;
                last_user = malloc(sizeof(user_list));
                last_user->previous_user = current_user;
                last_user->next_user = NULL;
                strcpy(last_user->user_name, msg.user_name);
                strcpy(last_user->user_msgq, msg.data);
                last_user->user_fd = mq_open(last_user->user_msgq, O_WRONLY | O_NONBLOCK);
                if(last_user->user_fd < 0) {
                    printf("Can not open %s\n", last_user->user_msgq);
                    free(last_user);
                    last_user = current_user;
                    if(last_user)
                        last_user->next_user = NULL;                
                }
                else {
                    sprintf(msg.data, "entered the chat");
                    if(current_user)
                        current_user->next_user = last_user;
                }
            }

            if(!strcmp(msg.data, EXIT_WORD)) {
                memset(msg.data, 0, sizeof(msg.data));
                sprintf(msg.data, "exit the chat");
                if(current_user == last_user)
                    last_user = last_user->previous_user;    
                del_user_from_list(current_user);         
            }

            for(current_user = last_user; current_user != NULL; 
                current_user = current_user->previous_user) {
                fflush(stdout);
                strcpy(msg_buf, msg.user_name);
                strtok(msg_buf, "0");
                strcat(msg_buf, msg.data);
                if(mq_send(current_user->user_fd, msg_buf, strlen(msg_buf), DEFAULT_PRIORITY) == -1) {
                    if(errno != EAGAIN) {
                        if(current_user == last_user)
                            last_user = last_user->previous_user;
                        printf("Can not send %s\nmsgq will be closed\n", msg_buf);
                        fflush(stdout);
                        del_user_from_list(current_user);
                    }
                }
            } 
        }
        else if(errno != EAGAIN) {  
            perror("mq_receive() "); 
            break;        
        }
        
        if(exit_flag)
            break;
    }

    pthread_join(thread_input, (void **)&thread_status);

    while(last_user != NULL) {
        close(last_user->user_fd);
        current_user = last_user;
        last_user = last_user->previous_user;
        free(current_user);
    }

    close(fd_read); 

    mq_unlink(SERWER_QUEUE);

    exit(EXIT_SUCCESS);
}

void del_user_from_list(user_list *user)
{
    user_list *next_user = user->next_user;
    user_list *previous_user= user->previous_user; 

    close(user->user_fd);
    if(previous_user)
        previous_user->next_user = next_user; 
    if(next_user)
        next_user->previous_user = previous_user;
    free(user);
}
