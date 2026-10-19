CC = gcc

CFLAGS = -Wall 

TARGET = main

SRCS = main.c defs.c game.c engine.c

OBJS = $(SRCS:.c=.o)

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)
	
