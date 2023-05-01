#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pipe_mkfifo.h"

int get_user_input_string(char *, int);

int main()
{
    pid_t child_pid;
    int status;
    int pipefd[5][ALL];
    char input_buf[500];
    char cmd_path[20];
    char *cmd[5][5];
    int i, j, first_proc = 0, last_proc = 0;

    while(1) {
        printf("введите команду или 'exit'\n");
        get_user_input_string(input_buf, sizeof(input_buf));
        if(!strcmp(input_buf, "exit"))
            exit(EXIT_SUCCESS);
        
        i = 0;
        cmd[i][0] = strtok(input_buf, "|"); 
        while(cmd[i][0] != NULL) {
            i++;
            cmd[i][0] = strtok(NULL, "|");
        }
        last_proc = i - 1;
        while(i--) {
            j = 0;
            cmd[i][j] = strtok(cmd[i][0], " \"");
            while(cmd[i][j] != NULL) {
                j++;
                cmd[i][j] = strtok(NULL, " \"");             
            }
        } 

        for(i = 0; i <= last_proc; i++) {            
            if(i < last_proc){
                if (pipe(pipefd[i]) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }

            child_pid = fork();
            if(child_pid == 0) {
                strcpy(cmd_path, DEF_BIN_PATH);
                strcat(cmd_path, cmd[i][0]);

                if(i < last_proc) {
                    close(pipefd[i][READ]);
                    dup2(pipefd[i][WRITE], WRITE);
                }
                if(i > first_proc) {
                    close(pipefd[i-1][WRITE]);
                    dup2(pipefd[i-1][READ], READ);
                }
                execv(cmd_path, cmd[i]);
                exit(EXIT_SUCCESS);
            }
            wait(&status);
            if(i < last_proc)
                close(pipefd[i][WRITE]);
        }

        for(i = 0; i < last_proc; i++) {
            close(pipefd[i][READ]);
        }
    }
    
    exit(EXIT_SUCCESS);
}


int get_user_input_string(char *str, int max_string_size)
{
    int str_lenght = 0, valid_itteraton = 1;

    while(fgets(str, max_string_size, stdin) != NULL){
        str_lenght = strlen(str);
        valid_itteraton--;
        if(str[str_lenght - 1] == '\n'){
            if(!valid_itteraton && str_lenght > 1){
                str[str_lenght - 1] = '\0';
                return str_lenght;
            }
            else{
                printf("Введите от 1 до %d символов\n", max_string_size-2);
                memset(str, 0, max_string_size);
                valid_itteraton = 1;
            }
        }
    }
    return 0;
}
