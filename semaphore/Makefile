

CC = gcc
BUILD_DIR = build

SRC = \
src/hello.c \
src/hi.c \
src/chat.c \
src/get_str.c 

EXE = \
$(BUILD_DIR)/hello \
$(BUILD_DIR)/hi \
$(BUILD_DIR)/chat

CINC = \
-Iinc

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
