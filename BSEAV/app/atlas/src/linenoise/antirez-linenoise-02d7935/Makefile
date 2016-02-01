linenoise_example: linenoise.h linenoise.c

CC = mipsel-linux-uclibc-gcc
linenoise_example: linenoise.c example.c
	$(CC) -Wall -W -Os -g -o linenoise_example linenoise.c example.c

clean:
	rm -f linenoise_example
