SOURCES:=$(wildcard ./src/*.c)
OBJS:=$(patsubst ./src/%.c,./src/.obj/%.o,$(SOURCES))
C_FLAGS=-std=c99 -g
EXECUTABLE=./dist/test

all: $(OBJS)
	cc -o $(EXECUTABLE) $^ $(C_FLAGS)

run:
	$(EXECUTABLE)

clean:
	rm $(OBJS)
	rm $(EXECUTABLE)

memcheck:
	valgrind --leak-check=full --show-leak-kinds=all $(EXECUTABLE)

.PHONY: all run clean memcheck

./src/.obj/%.o: ./src/%.c
	-cc -c -o $@ $< -I./include $(C_FLAGS)