#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "pipe_mkfifo.h"

// #define FIFO_SERWER "./fifo_to_server"
// #define FIFO_CLIENT "./fifo_to_client"

// enum pipe_direction{
//     READ = 0,
//     WRITE,
//     ALL
// };

int main()
{

    char msg_str[10];
    char readed_byte;
    int offset = 0;
    int fd_read, fd_write; 

    if (mkfifo(FIFO_SERWER, S_IRWXU) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(FIFO_CLIENT, S_IRWXU) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    fd_write = open(FIFO_CLIENT, O_WRONLY);
    if(fd_write < 0) {
        perror("open");
        exit(EXIT_FAILURE);        
    }

    fd_read = open(FIFO_SERWER, O_RDONLY);
    if(fd_read < 0) {
        perror("open");
        exit(EXIT_FAILURE);        
    }   

    do {
        if( read(fd_read, &readed_byte, 1) > 0 )
            msg_str[offset++] = readed_byte;    
    }while(readed_byte != '\n');
    msg_str[offset] = '\0';
    printf("%s", msg_str); 

    write(fd_write, "Hi\n", strlen("Hi\n"));

    close(fd_write);
    close(fd_read); 
    remove(FIFO_SERWER);  
    remove(FIFO_CLIENT); 

    exit(EXIT_SUCCESS);
}