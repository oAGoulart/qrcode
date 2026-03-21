.PHONY: default clean lint debug build

TARGET_EXEC := qrcode

CC := clang
CCFLAGS := -DNDEBUG -g0 -O3 -Wno-ignored-qualifiers -Wno-gnu-designator
LDFLAGS := -lm -no-pie -Wno-unused-command-line-argument

BUILD_DIR := ./bin
SCRIPT_DIR := ./scripts

SCPT := indexes.py

SRCS := vector.c bits.c bytes.c mask.c data.c code.c main.c
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

default: build

lint: lint_clang lint_cppcheck
lint_clang:
	clang-tidy $(SRCS) -checks=-*,performance-*,portability-*\
	-- -D__clang__
lint_cppcheck:
	cppcheck $(SRCS) -D__clang__ --force --enable=all\
	--suppress=missingIncludeSystem --suppress=unusedFunction

debug: debug_build
debug_build: CCFLAGS ::= -g3 -O0
debug_build: build

build: $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/$(TARGET_EXEC): $(BUILD_DIR)/lookup.o $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
	-Wno-gnu-zero-variadic-macro-arguments \
	--std=gnu11 $(CCFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/lookup.o: lookup.S | $(BUILD_DIR)
	$(CC) -c lookup.S -o $@ -save-temps

$(BUILD_DIR):
	mkdir -p $@

lookup.S: $(SCRIPT_DIR)/lookup.sh $(SCPT:%=$(SCRIPT_DIR)/%)
	chmod 777 $(SCRIPT_DIR)/lookup.sh $(SCPT:%=$(SCRIPT_DIR)/%) ; \
	rm -f lookup.S ; \
	sh $(SCRIPT_DIR)/lookup.sh

clean:
	rm -rf $(BUILD_DIR) ; \
	rm -f lookup.S lookup.s

-include $(DEPS)
