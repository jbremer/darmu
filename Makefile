CC = gcc
CFLAGS = -Wall -std=c99 -s -O2 -Wextra -Wno-unused-parameter
DIRS = -Idarm -Ldarm
LIBS = $(wildcard darm/*.o)

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

FILES = $(OBJ) libdarmu.a libdarmu.so

default: $(FILES)

%.o: %.c
	$(CC) $(CFLAGS) $(DIRS) -o $@ -c $^

%.so: $(OBJ) $(LIBS)
	$(CC) -shared $(CFLAGS) $(DIRS) -o $@ $^

%.a: $(OBJ) $(LIBS)
	ar cr $@ $^

clean:
	rm -rf $(FILES)
