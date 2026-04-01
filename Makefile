CC          := gcc
CFLAGS      := -Wall -Iinclude -MMD -MP
LDFLAGS     := -lpthread

# Directories
BIN_DIR     := bin

# Source files
SERVER_SRCS := $(wildcard src/*.c)
CLIENT_SRCS := $(wildcard client/*.c) src/net.c  //notes ve viet lai

# Object files
SERVER_OBJS := $(SERVER_SRCS:.c=.o)
CLIENT_OBJS := $(CLIENT_SRCS:.c=.o)

# Dependency files
SERVER_DEPS := $(SERVER_OBJS:.o=.d)
CLIENT_DEPS := $(CLIENT_OBJS:.o=.d)

# Output binaries
SERVER_BIN  := $(BIN_DIR)/server_test
CLIENT_BIN  := $(BIN_DIR)/client_test

all: server client

server: $(SERVER_BIN)
client: $(CLIENT_BIN)

# server
$(SERVER_BIN): $(SERVER_OBJS) | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

# client
$(CLIENT_BIN): $(CLIENT_OBJS) | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

-include $(SERVER_DEPS) $(CLIENT_DEPS)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS)
	rm -f $(SERVER_DEPS) $(CLIENT_DEPS)

.PHONY: all server client clean