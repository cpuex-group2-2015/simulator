CC = gcc
LD = gcc
CFLAGS = -Wall -MMD -MP
LDFLAGS =
TARGET = ./bin/sim
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: all clean
