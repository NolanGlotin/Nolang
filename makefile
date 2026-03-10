CC = gcc
FLAGS = -Wall -Werror -fsanitize=address

all: main

main: main.c
	$(CC) main.c -o nolang $(FLAGS)