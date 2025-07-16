TARGET_EXEC := qrcode

CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 --std=c11 -g0 -O3
LDFLAGS := -lm

BUILD_DIR := ./bin
SRCS := mask.c quickresponse.c main.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)
