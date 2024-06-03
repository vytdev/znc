CC 	= gcc
CFLAGS 	= -std=c99 -Wall -pedantic -MMD -MP
LDFLAGS =
SRC 	= $(shell find . -type f -name '*.c')
OBJ 	= $(SRC:.c=.o)
DEP 	= $(SRC:.c=.d)
TARGET  = znc

CFLAGS += -D _ZNC_VERSION=$(shell cat VERSION)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ)
	rm -rf $(DEP)
	rm -rf $(TARGET)

debug: CFLAGS  += -g -D _DEBUG
debug: LDFLAGS += -g
debug: all

perf: CFLAGS += -O3 -march=native -mtune=native -fomit-frame-pointer \
	-funroll-loops -finline-functions
perf: all

-include $(DEP)

.PHONY: clean debug perf

