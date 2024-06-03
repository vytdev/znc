CC 	= gcc
CFLAGS 	= -std=c99 -Wall -Werror -pedantic -g
INCLUDE =
LIBS    =

SRCS  = $(shell find . -type f -name '*.c' ! -path './__test.c')
TESTS = $(basename $(SRCS))

.PHONY: all clean $(TESTS)
all: $(TESTS)
clean:
	rm -rf $(TESTS)
$(TESTS):
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -D _TESTSUITE='"$@"' __test.c $@.c -o $@ $($@)
	./$@

# find source files
SDIR = $(filter-out ../src/main.c,$(shell find ../src -type f -name '*.c'))
LIBS += $(SDIR:.c=.o)

