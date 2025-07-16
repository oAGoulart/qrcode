TARGET_EXEC := qrcode

CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 --std=c11 -g0 -O3
LDFLAGS := -lm

BUILD_DIR := ./bin
SRCS := mask.c quickresponse.c main.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET_EXEC): lookup.o $(OBJS)
	$(CC) $(BUILD_DIR)/lookup.o $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lookup.o:
	mkdir -p $(BUILD_DIR)
	$(CC) -c lookup.S -o $(BUILD_DIR)/lookup.o

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)
