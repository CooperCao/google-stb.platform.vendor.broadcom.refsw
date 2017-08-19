#include <linux/config.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/poll.h>

#include "rstest_cmd.h"

#include "ifconfig.h"
#include "ttcp.h"

#define TOKENIZE_MAX		16

static int
tokenize_cmdline(char *cmd_line, char *argv[])
{
	char *s = cmd_line, *t;
	int qquote = 0;
	int argc = 0;

	/* Consumes input line by overwriting spaces with NULs */

	while (argc < TOKENIZE_MAX - 1) {
		while (*s == ' ')
			s++;
		if (!*s)
			break;
		argv[argc++] = s;
		while (*s && (qquote || *s != ' ')) {
			if (*s == '"') {
				qquote ^= 1;
				for (t = s--; (t[0] = t[1]) != 0; t++)
					;
			}
			s++;
		}
		if (*s)
			*s++ = 0;
	}

	argv[argc] = NULL;

	return argc;
}

int
rstest_cmd(char *cmd)
{
	char *argv[TOKENIZE_MAX];
	int argc;

	argc = tokenize_cmdline(cmd, argv);

	if (argc > 0) {
		if (strcmp(argv[0], "ifconfig") == 0)
			ifconfig(argc, argv);
		else if (strcmp(argv[0], "ttcp") == 0)
			ttcp(argc, argv, 0);
		else
			return -1;
	}

	return 0;
}
