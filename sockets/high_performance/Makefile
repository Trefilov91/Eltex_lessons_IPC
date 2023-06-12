

CC = gcc
BUILD_DIR = build
FUNC_DIR = src/func

SRC = \
src/inet_tcp_client.c \
src/inet_udp_client.c \
src/1_work_thread_server.c \
src/5_work_thread_server.c \
src/server_with_queue.c \
src/multi_server.c \
src/func/get_str.c \
src/func/list.c

CINC = \
-Iinc

OBJ_TEMP = $(addprefix $(BUILD_DIR)/,$(notdir $(filter-out $(FUNC_DIR)/%,$(SRC:.c=.o))))
OBJ_FUNC = $(addprefix $(BUILD_DIR)/,$(notdir $(filter $(FUNC_DIR)/%,$(SRC:.c=.o))))
EXE = $(addprefix $(BUILD_DIR)/,$(notdir $(OBJ_TEMP:.o=)))
OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))
vpath %.c $(dir $(SRC))


all: $(BUILD_DIR) $(OBJ) $(EXE)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(OBJ): $(BUILD_DIR)/%.o: %.c
	$(CC) $(CINC) -c $< -o $@

$(EXE): $(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CC) $(CINC) $< $(OBJ_FUNC) -o $@	

clean:
	rm -rf $(BUILD_DIR)
