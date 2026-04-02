CC          := gcc
CFLAGS      := -Wall -Iinclude -MMD -MP
LDFLAGS     := -lpthread

# Directories
BIN_DIR     := bin
BUILD_DIR   := build

# Source files
SERVER_SRCS := $(wildcard src/*.c)
CLIENT_SRCS := $(wildcard client/*.c) src/net.c

# Object files
SERVER_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SERVER_SRCS))
CLIENT_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(CLIENT_SRCS))

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

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

-include $(SERVER_DEPS) $(CLIENT_DEPS)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
	rm -rf $(BUILD_DIR)

.PHONY: all server client clean
