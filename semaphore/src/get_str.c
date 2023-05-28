#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm.h"

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
