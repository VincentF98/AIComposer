CC := gcc

C_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
LD_FLAGS := -lm
debug: CFLAGS := -g -O0 -Wall
release: CFLAGS := -O2 -Wall

All: debug

release: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o ./bin/composer $^

debug: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o ./bin/composer $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf ./bin/composer
	rm -rf ./obj/*.o
