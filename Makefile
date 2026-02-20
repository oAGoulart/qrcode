.PHONY: default
default: build

TARGET_EXEC := qrcode

CC := clang
CCFLAGS := -DNDEBUG -g0 -O3
LDFLAGS := -lm

BUILD_DIR := ./bin
SCRIPT_DIR := ./scripts

SCPT := indexes.py

SRCS := bits.c bytes.c mask.c data.c code.c main.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

lint: lint_clang lint_cppcheck
lint_clang:
	clang-tidy $(SRCS) -checks=-*,performance-*,portability-* -- -D__clang__
lint_cppcheck:
	cppcheck $(SRCS) -D__clang__ --force --enable=all --suppress=missingIncludeSystem --suppress=unusedFunction

debug: debug_build
debug_build: CCFLAGS ::= -g3 -O0
debug_build: build

build: lookup.o $(OBJS)
	$(CC) $(BUILD_DIR)/lookup.o $(OBJS) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	$(CC) -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
	-Wno-gnu-zero-variadic-macro-arguments \
	--std=gnu11 $(CCFLAGS) -c $< -o $@

lookup.o: lookup.S
	mkdir -p $(BUILD_DIR) ; \
	$(CC) -c lookup.S -o $(BUILD_DIR)/lookup.o -save-temps

lookup.S:
	chmod 777 $(SCRIPT_DIR)/lookup.sh $(SCPT:%=$(SCRIPT_DIR)/%) ; \
	rm -f lookup.S ; \
	bash -c $(SCRIPT_DIR)/lookup.sh

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)
