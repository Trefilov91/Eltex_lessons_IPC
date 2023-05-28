#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include "shm.h"
#include <pthread.h>

chat_shm_t *chat_shm;
sem_t *shm_sem;
char user_name[USER_NAME_MAX_LEN];
unsigned char exit_flag = 0;

void *thread_user_input(void *args)
{
    char input_buf[MSG_DATA_MAX_LEN];
    int msg_index;
    printf("введите '%s' для выхода\n", EXIT_WORD);
    while(1) {
        memset(input_buf, 0, sizeof(input_buf));
        get_user_input_string(input_buf, sizeof(input_buf));  
        sem_wait(shm_sem);    
        msg_index = chat_shm->next_msg_index;
        strcpy(chat_shm->msg[msg_index].user_name, user_name);
        if(!strcmp(input_buf, EXIT_WORD)) {
            exit_flag = 1;
            strcpy(chat_shm->msg[msg_index].data, "exit the chat");
        }            
        else {
            strcpy(chat_shm->msg[msg_index].data, input_buf); 
        }  
        chat_shm->next_msg_index++;
        if(chat_shm->next_msg_index == MSG_DATA_MAX_NUM)  
            chat_shm->next_msg_index = 0;
        sem_post(shm_sem);
        if(exit_flag)
            break;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int fd_shm;    
    pthread_t thread_input;
    int *thread_status;
    int print_msg_mum = 0;
    int last_print_msg_mum = 0;
    int i;

    if(argc != 3) {
        printf("Print your name and sername as atribut\n");
        exit(EXIT_FAILURE);
    }

    memset(user_name, 0, sizeof(user_name));
    sprintf(user_name, "%s %s", argv[1], argv[2]);


    fd_shm = shm_open(CHAT_SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0777);
    if(fd_shm < 0) {
        if(errno == EEXIST) {
            fd_shm = shm_open(CHAT_SHM_NAME, O_RDWR, 0777);
            if(fd_shm < 0) {
                perror("shm_open");
                exit(EXIT_FAILURE);
            }
            chat_shm = mmap(0, sizeof(chat_shm_t), PROT_WRITE|PROT_READ, MAP_SHARED, fd_shm, 0);
        }
        else {
            perror("shm_open");
            exit(EXIT_FAILURE);
        }
    }  
    else {
        if ( ftruncate(fd_shm, sizeof(chat_shm_t)) == -1 ) {
            perror("ftruncate");
            close(fd_shm);
            shm_unlink(CHAT_SHM_NAME);
            exit(EXIT_FAILURE);
        } 
        chat_shm = mmap(0, sizeof(chat_shm_t), PROT_WRITE|PROT_READ, MAP_SHARED, fd_shm, 0);
        chat_shm->next_msg_index = 0;
    }   

    shm_sem = sem_open(CHAT_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0777, 0);
    if(shm_sem == SEM_FAILED ) {
        if(errno == EEXIST) {
            shm_sem = sem_open(CHAT_SEMAPHORE_NAME, 0);
            if(shm_sem == SEM_FAILED ) {
                perror("sem_open");
                munmap(chat_shm, MSG_DATA_MAX_LEN); 
                close(fd_shm);
                shm_unlink(CHAT_SHM_NAME);
                exit(EXIT_FAILURE);
            }
        }
        else {
            perror("sem_open");
            munmap(chat_shm, MSG_DATA_MAX_LEN); 
            close(fd_shm);
            shm_unlink(CHAT_SHM_NAME);
            exit(EXIT_FAILURE);
        }
    }
    else
        sem_post(shm_sem);

    pthread_create(&thread_input, NULL, thread_user_input, NULL);

    while(!exit_flag) {
        sem_wait(shm_sem);
        for(i = 0; i < 2; i++) {
            if(print_msg_mum != chat_shm->next_msg_index) {
                last_print_msg_mum = print_msg_mum < chat_shm->next_msg_index ? chat_shm->next_msg_index : MSG_DATA_MAX_NUM;
                while(print_msg_mum < last_print_msg_mum) {
                    printf("%s: %s\n", chat_shm->msg[print_msg_mum].user_name, chat_shm->msg[print_msg_mum].data);
                    print_msg_mum++;                    
                }
                if(print_msg_mum == MSG_DATA_MAX_NUM)
                    print_msg_mum = 0;
            }
        }
        sem_post(shm_sem);
    }

    pthread_join(thread_input, (void **)&thread_status);
    munmap(chat_shm, sizeof(chat_shm_t)); 
    close(fd_shm);
    shm_unlink(CHAT_SHM_NAME);
    sem_close(shm_sem);
    sem_unlink(CHAT_SEMAPHORE_NAME);
    exit(EXIT_SUCCESS);
}
