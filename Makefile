CC = gcc
LD = gcc
CFLAGS = -Wall -MMD -MP
LDFLAGS =
TARGET = ./bin/sim
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TESTTXT = $(wildcard ./test/*.txt)
TESTBIN = $(TESTTXT:.txt=.bin)

all: $(TARGET) $(TESTBIN)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

./test/%.bin: ./test/%.txt
	./script/txt2bin.py $< $@
clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: all clean
