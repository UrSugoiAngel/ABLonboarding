CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lncurses

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
TARGET = ablob

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean