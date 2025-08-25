.PHONY: default
default: build

TARGET_EXEC := qrcode

CC := clang
CCFLAGS := -DNDEBUG -g0 -O3
LDFLAGS := -lm

BUILD_DIR := ./bin
SRCS := packedbits.c heaparray.c mask.c quickresponse.c main.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

lint: lint_clang lint_cppcheck
lint_clang:
	clang-tidy main.c
lint_cppcheck:
	cppcheck main.c

debug: debug_build
debug_build: CCFLAGS ::= -g3 -O0
debug_build: build

build: lookup.o $(OBJS)
	$(CC) $(BUILD_DIR)/lookup.o $(OBJS) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	$(CC) -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wno-gnu-zero-variadic-macro-arguments --std=gnu11 $(CCFLAGS) -c $< -o $@

lookup.o: lookup.S
	mkdir -p $(BUILD_DIR)
	$(CC) -c lookup.S -o $(BUILD_DIR)/lookup.o

lookup.S:
	bash -c scripts/lookup.sh

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)
