CC = gcc
CFLAGS = $(shell sdl-config --cflags --libs) -lSDL_image
#LDFLAGS += -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer

TARGET = sdl_sim

SRCS = main.c
SRCS += sbuf.c

OBJS = $(SRCS:%.c=%.o)

all: $(TARGET)

sdl_sim: $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) -lpthread

app: run.o
	$(CC) $^ -o $@ $(CFLAGS)

run: $(TARGET)
	./$<

.PHONY: tags
tags:
	ctags *.c *.h

%.o: %.c
	$(CC) -c $< $(CFLAGS) 

%.o: %.cpp
	$(CC) -c $< $(CFLAGS) 

clean:
	$(RM) -rf *.o $(TARGET) tags

.PHONY: all run clean
