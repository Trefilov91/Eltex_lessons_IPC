

CC = gcc
BUILD_DIR = build

SRC = \
src/pipe_helo.c \
src/mkfifo_server.c \
src/mkfifo_client.c \
src/interpritator.c

CINC = \
-Iinc

EXE = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=)))
vpath %.c $(dir $(SRC))

all: $(BUILD_DIR) $(EXE)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(EXE): $(BUILD_DIR)/%: %.c
	$(CC) $(CINC) $< -o $@

clean:
	rm -rf $(BUILD_DIR)
