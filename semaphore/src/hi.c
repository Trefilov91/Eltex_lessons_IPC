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

    fd_shm = shm_open(CHAT_SHM_NAME, O_RDWR, 0777);
    if(fd_shm < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }  

    shm_addr = mmap(0, MSG_DATA_MAX_LEN, PROT_WRITE|PROT_READ, MAP_SHARED, fd_shm, 0);

    shm_sem = sem_open(CHAT_SEMAPHORE_NAME, 0);
    if(shm_sem == SEM_FAILED ) {
        perror("sem_open");
        munmap(shm_addr, MSG_DATA_MAX_LEN); 
        close(fd_shm);
        shm_unlink(CHAT_SHM_NAME);
        exit(EXIT_FAILURE);
    }

    while(1) {
        sem_wait(shm_sem);
        if(shm_addr[0] == 0) {
            sem_post(shm_sem);   
        }
        else {
            printf("%s", shm_addr);
            strcpy(shm_addr, "hi\n");
            sem_post(shm_sem); 
            munmap(shm_addr, MSG_DATA_MAX_LEN); 
            close(fd_shm);
            shm_unlink(CHAT_SHM_NAME);
            sem_close(shm_sem);
            exit(EXIT_SUCCESS);            
        }
    }
}



