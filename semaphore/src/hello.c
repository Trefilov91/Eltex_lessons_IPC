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

int main()
{
    sem_t *shm_sem;
    int fd_shm;
    char *shm_addr;

    fd_shm = shm_open(CHAT_SHM_NAME, O_CREAT | O_RDWR, 0777);
    if(fd_shm < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }  

    if ( ftruncate(fd_shm, MSG_DATA_MAX_LEN) == -1 ) {
        perror("ftruncate");
        close(fd_shm);
        shm_unlink(CHAT_SHM_NAME);
        exit(EXIT_FAILURE);
    }

    shm_addr = mmap(0, MSG_DATA_MAX_LEN, PROT_WRITE|PROT_READ, MAP_SHARED, fd_shm, 0);
    memset(shm_addr, 0, MSG_DATA_MAX_LEN);
    strcpy(shm_addr, "Hello\n");

    shm_sem = sem_open(CHAT_SEMAPHORE_NAME, O_CREAT, 0777, 0);
    if(shm_sem == SEM_FAILED ) {
        perror("sem_open");
        munmap(shm_addr, MSG_DATA_MAX_LEN); 
        close(fd_shm);
        shm_unlink(CHAT_SHM_NAME);
        exit(EXIT_FAILURE);
    }
    sem_post(shm_sem);

    while(1) {
        sem_wait(shm_sem);
        if(!strcmp(shm_addr, "Hello\n")) {
            sem_post(shm_sem);
        }
        else {
            printf("%s", shm_addr);
            munmap(shm_addr, MSG_DATA_MAX_LEN); 
            close(fd_shm);
            shm_unlink(CHAT_SHM_NAME);
            sem_close(shm_sem);
            sem_unlink(CHAT_SEMAPHORE_NAME);
            exit(EXIT_SUCCESS);
        }
    }
}


