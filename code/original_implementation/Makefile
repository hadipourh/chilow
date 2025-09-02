FLAGS=-O3 -Wall -Wextra
CC=gcc
BUILD_DIR := ./build
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

# Exclude integral_verification from main build
MAIN_SRCS = $(filter-out integral_verification.c, $(wildcard *.c))
MAIN_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(MAIN_SRCS))

# Integral verification sources (excluding main.c)
INTEGRAL_SRCS = kiki.c utility.c integral_verification.c
INTEGRAL_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(INTEGRAL_SRCS))

all: main

depend:
	makedepend $(SRCS)

# Build step for C source
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

# The final build step for main
main : $(MAIN_OBJS)
	$(CC) $(FLAGS) $^ -o $@

# Build integral verification tool
integral_verification : $(INTEGRAL_OBJS)
	$(CC) $(FLAGS) $^ -o $@

# Run integral verification
.PHONY: test_integral
test_integral: integral_verification
	@echo "Running integral verification with original implementation..."
	./integral_verification

clean:
	rm -f main integral_verification
	rm -rf $(BUILD_DIR)




# DO NOT DELETE

kiki.o: utility.h chichi.h linearLayer.h
main.o: kiki.h
main.o: utility.h
utility.o: kiki.h
