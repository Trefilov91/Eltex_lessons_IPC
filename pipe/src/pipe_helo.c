#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pipe_mkfifo.h"

// enum pipe_direction{
//     READ = 0,
//     WRITE,
//     ALL
// };

int main()
{
    pid_t child_pid;
    int status;
    int pipefd1[ALL];
    int pipefd2[ALL];
    char msg_str[10];
    char readed_byte;
    int offset = 0;

    if (pipe(pipefd1) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipefd2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    child_pid = fork();
    if(child_pid == 0) {
        close(pipefd1[WRITE]);   
        close(pipefd2[READ]);       
        do {
            if( read(pipefd1[READ], &readed_byte, 1) > 0 )
                msg_str[offset++] = readed_byte;    
        }while(readed_byte != '\n');
        msg_str[offset] = '\0';
        printf("%s", msg_str);
        strcpy(msg_str, "Hi\n");
        write(pipefd2[WRITE], msg_str, strlen(msg_str));

        close(pipefd1[READ]);
        close(pipefd2[WRITE]);
        exit(EXIT_SUCCESS);
    }
    else {
        close(pipefd1[READ]); 
        close(pipefd2[WRITE]);  
        strcpy(msg_str, "Hello\n");
        write(pipefd1[WRITE], msg_str, strlen(msg_str));
        do {
            if( read(pipefd2[READ], &readed_byte, 1) > 0 )
                msg_str[offset++] = readed_byte;    
        }while(readed_byte != '\n');
        msg_str[offset] = '\0';
        printf("%s", msg_str);
        close(pipefd1[WRITE]);
        close(pipefd2[READ]);
        wait(&status);
    }

    exit(EXIT_SUCCESS);
}