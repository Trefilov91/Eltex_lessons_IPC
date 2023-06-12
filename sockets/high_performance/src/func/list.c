#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "socket.h"

user_list *del_user_from_list(user_list *user)
{
    user_list *next_user = user->next_user;
    user_list *previous_user= user->previous_user; 

    close(user->user_fd);
    if(previous_user)
        previous_user->next_user = next_user; 
    if(next_user)
        next_user->previous_user = previous_user;
    free(user);

    return previous_user;
}