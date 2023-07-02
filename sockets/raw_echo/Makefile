

CC = gcc
BUILD_DIR = build
FUNC_DIR = src/func

SRC = \
src/inet_udp_server.c \
src/raw_1thread_client.c \
src/raw_2thread_client.c \
src/func/get_str.c

CINC = \
-Iinc

OBJ_TEMP = $(addprefix $(BUILD_DIR)/,$(notdir $(filter-out $(FUNC_DIR)/%,$(SRC:.c=.o))))
EXE = $(addprefix $(BUILD_DIR)/,$(notdir $(OBJ_TEMP:.o=)))
OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))
vpath %.c $(dir $(SRC))


all: $(BUILD_DIR) $(OBJ) $(EXE)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(OBJ): $(BUILD_DIR)/%.o: %.c
	$(CC) $(CINC) -c $< -o $@

$(EXE): $(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CC) $(CINC) $< $(BUILD_DIR)/get_str.o -o $@	

clean:
	rm -rf $(BUILD_DIR)
