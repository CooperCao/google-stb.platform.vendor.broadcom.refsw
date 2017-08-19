INCLUDES = -I.

CFLAGS += $(INCLUDES)

SRCS = parser.c
OBJS = $(SRCS:.c=.o)

all: parser

parser: $(SRCS)
	$(CC) -o $@ $(CFLAGS) $(SRCS)

.PHONY: clean
clean:
	$(RM) *.o *.d parser
