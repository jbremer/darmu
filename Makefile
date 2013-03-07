CC = gcc
CFLAGS = -Wall -std=c99 -s -O2 -Idarm -Ldarm
LIBS = -ldarm

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

FILES = $(OBJ) libdarmu.so

default: $(FILES)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

%.so: $(OBJ)
	$(CC) -shared $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(FILES)
