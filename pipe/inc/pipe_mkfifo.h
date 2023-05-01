#ifndef PIPE_H
#define PIPE_H

#define FIFO_SERWER "/tmp/fifo_to_server"
#define FIFO_CLIENT "/tmp/fifo_to_client"
#define DEF_BIN_PATH "/bin/"

enum pipe_direction{
    READ = 0,
    WRITE,
    ALL
};

#endif /*PIPE_H*/