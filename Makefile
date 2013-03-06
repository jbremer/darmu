CC = gcc
CFLAGS = -Wall -std=c99 -Idarm -Ldarm
LIBS = -ldarm

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

default: $(OBJ) libdarm.so

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

%.so: $(OBJ)
	$(CC) -shared $(CFLAGS) -o $@ $^ $(LIBS)
