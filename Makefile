CC = gcc
LD = gcc
CFLAGS = -Wall -MMD -MP -O3
LDFLAGS = -lm
TARGET = ./bin/sim
SRCDIR = ./src
BUILDDIR = ./build
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=$(BUILDDIR)/%.o)
DEPS = $(SRCS:%.c=$(BUILDDIR)/%.d)

TESTTXT = $(wildcard ./test/*.txt)
TESTBIN = $(TESTTXT:.txt=.bin)

all: $(TARGET) $(TESTBIN)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

./test/%.bin: ./test/%.txt
	./script/txt2bin.py $< $@
clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) $(TESTBIN)

.PHONY: all clean
