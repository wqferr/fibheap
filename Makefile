SOURCES:=$(wildcard ./src/*.c)
OBJS:=$(patsubst ./src/%.c,./src/.obj/%.o,$(SOURCES))
C_FLAGS=-std=c99 -g -Wextra -Wall -Werror
CC=clang
EXECUTABLE=./dist/test

all: $(OBJS)
	$(CC) -o $(EXECUTABLE) $^ $(C_FLAGS)

run:
	$(EXECUTABLE)

clean:
	rm $(OBJS)
	rm $(EXECUTABLE)

memcheck:
	valgrind --leak-check=full --show-leak-kinds=all $(EXECUTABLE)

.PHONY: all run clean memcheck

./src/.obj/%.o: ./src/%.c
	-$(CC) -c -o $@ $< -I./include $(C_FLAGS)