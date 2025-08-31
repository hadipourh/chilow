FLAGS=-O3 -Wall -Wextra
CC=gcc
BUILD_DIR := ./build
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: main

depend:
	makedepend $(SRCS)

# Build step for C source
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

# The final build step.
main : $(OBJS)
	$(CC) $(FLAGS) $^ -o $@

clean:
	rm main
	rm -r $(BUILD_DIR)




# DO NOT DELETE

kiki.o: utility.h chichi.h linearLayer.h
main.o: kiki.h
main.o: utility.h
utility.o: kiki.h
