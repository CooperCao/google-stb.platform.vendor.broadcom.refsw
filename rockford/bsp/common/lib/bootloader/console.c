/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include <string.h>
#include <stdarg.h>
#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"

#include "iocb.h"
#include "devfuncs.h"
#include "timer.h"
#include "error.h"
#include "env_subr.h"
#include "console.h"
#include "bolt.h"

/*
Escape sequences:

Sequence       Descr   Emulator

ESC [ A        UP      xterm
ESC [ B        DOWN    xterm
ESC [ C        RIGHT   xterm
ESC [ D        LEFT    xterm

ESC O P        F1      xterm
ESC O Q        F2      xterm
ESC O R        F3      xterm
ESC O S        F4      xterm

ESC [ 1 1 ~    F1      teraterm
ESC [ 1 2 ~    F2      teraterm
ESC [ 1 3 ~    F3      teraterm
ESC [ 1 4 ~    F4      teraterm

ESC [ 1 5 ~    F5      xterm,teraterm
ESC [ 1 7 ~    F6      xterm,teraterm
ESC [ 1 8 ~    F7      xterm,teraterm
ESC [ 1 9 ~    F8      xterm,teraterm
ESC [ 2 0 ~    F9      xterm,teraterm
ESC [ 2 1 ~    F10     xterm,teraterm
ESC [ 2 3 ~    F11     xterm,teraterm
ESC [ 2 4 ~    F12     xterm,teraterm

ESC [ 5 ~      PGUP    xterm
ESC [ 6 ~      PGDN    xterm
ESC [ F        END     xterm
ESC [ H        HOME    xterm

ESC [ 2 ~      HOME    teraterm
ESC [ 3 ~      PGUP    teraterm
ESC [ 4 ~      DEL     teraterm
ESC [ 5 ~      END     teraterm
ESC [ 6 ~      PGDN    teraterm

ESC [ 3 ~      DEL     gnome-terminal
ESC [ 5 ~      PGUP    gnome-terminal
ESC [ 6 ~      PGDN    gnome-terminal
ESC O F        HOME    gnome-terminal
ESC O H        END     gnome-terminal

*/

/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#define CTRL(x) ((x)-'@')

#define GETCHAR(x)	do {\
				while (console_read(&(x), 1) != 1) \
					POLL();\
			} while (0)

#define XTERM		0
#define TERATERM	1

#define MAXSAVELINES	30
#define MSGQUEUEMAX	 10	/* number of chunks of log to keep */
#define MSGQUEUESIZE	256	/* size of each chunk of console log */

/*  *********************************************************************
    *  Structures
    ********************************************************************* */

typedef struct msgqueue_s {	/* saved console log message */
	queue_t link;
	int len;
	char data[MSGQUEUESIZE];
} msgqueue_t;

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

#if 0
#if !CFG_MIN_CONSOLE
int console_nextsave = 0;
char *console_savedlines[MAXSAVELINES] = { 0 };

static char *console_killbuffer = NULL;
queue_t console_msgq = { &console_msgq, &console_msgq };

static int console_buffer_flg = 0;
static int console_mode = XTERM;

static void console_flushbuffer(void);
static void console_save(/* arm unsigned */ char *buffer, int length);

#endif
#endif

int console_handle = -1;
int console_xprint(const char *str);
char *console_name = NULL;
static int console_inreadline = 0;
static int console_redisplay = 0;

extern int bcm97XXX_uart_write(char *bptr, int blen);

/*  *********************************************************************
    *  console_log(tmplt,...)
    *
    *  Sort of like printf, but used during things that might be
    *  called in the polling loop. If you print out a message
    *  using this call and it happens to be printed while processing
    *  the console "readline" loop, the readline and the associated
    *  prompt will be redisplayed.  Don't include the \r\n in the
    *  string to be displayed.
    *
    *  Input parameters:
    *      tmplt, ... - printf parameters
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void console_log(const char *tmplt, ...)
{
	char buffer[256];
	va_list marker;

	va_start(marker, tmplt);
	(void) xvsprintf(buffer, tmplt, marker);
	va_end(marker);
	xprintf("\r%s\033[J\r\n", buffer);

	if (console_inreadline)
		console_redisplay = 1;
}

#if 0
/*  *********************************************************************
    *  console_open(name)
    *
    *  Open the specified device and make it the console
    *
    *  Input parameters:
    *      name - name of device
    *
    *  Return value:
    *      0 if ok, else return code.
    *      console_handle contains the console's handle
    ********************************************************************* */

int console_open(char *name)
{
#if CFG_MIN_CONSOLE
	if (console_handle != -1)
		console_close();

	console_handle = bolt_open(name);
	if (console_handle < 0)
		return BOLT_ERR_DEVNOTFOUND;

	console_name = name;
#else

	int flushbuf;

	console_name = NULL;

	if (console_handle != -1)
		console_close();

	flushbuf = console_buffer_flg;
	console_buffer_flg = 0;

	console_handle = bolt_open(name);
	if (console_handle < 0)
		return BOLT_ERR_DEVNOTFOUND;

	console_name = name;
	if (flushbuf)
		console_flushbuffer();
#endif

	return 0;
}

/*  *********************************************************************
    *  console_close()
    *
    *  Close the console device
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      0
    ********************************************************************* */

int console_close(void)
{
	if (console_handle != -1)
		bolt_close(console_handle);

	console_handle = -1;

	return 0;
}

/*  *********************************************************************
    *  console_read(buffer,length)
    *
    *  Read characters from the console.
    *
    *  Input parameters:
    *      buffer - pointer to buffer to receive characters
    *      length - total size of the buffer
    *
    *  Return value:
    *      number of characters received, or <0 if error code
    ********************************************************************* */

int console_read(unsigned char *buffer, int length)
{
	if (console_handle == -1)
		return -1;

	return bolt_read(console_handle, buffer, length);
}
#endif

/*  *********************************************************************
    *  console_write(buffer,length)
    *
    *  Write text to the console.  If the console is not open and
    *  we're "saving" text, put the text on the in-memory queue
    *
    *  Input parameters:
    *      buffer - pointer to data
    *      length - number of characters to write
    *
    *  Return value:
    *      number of characters written or <0 if error
    ********************************************************************* */

int console_write(/* arm unsigned */ char *buffer, int length)
{
	int res;

#if 0
#if !CFG_MIN_CONSOLE
	/*
	 * Buffer text if requested
	 */
	if (console_buffer_flg) {
		console_save(buffer, length);
		return length;
	}
#endif

	/*
	 * Do nothing if no console
	 */

	if (console_handle == -1)
		return -1;
#endif

	/*
	 * Write text to device
	 */

	for (;;) {
		res =
		    bcm97XXX_uart_write(buffer, length);
		if (res < 0)
			break;
		buffer += res;
		length -= res;
		if (length == 0)
			break;
	}

	if (res < 0)
		return -1;
	return 0;
}

#if 0
/*  *********************************************************************
    *  console_status()
    *
    *  Return the status of input for the console.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      0 if no characters are available
    *      1 if characters are available.
    ********************************************************************* */

int console_status(void)
{
	if (console_handle == -1)
		return 0;

	return bolt_inpstat(console_handle);
}
#endif

/*  *********************************************************************
    *  console_xprint(str)
    *
    *  printf callback for the console device.  the "xprintf"
    *  routine ends up calling this.  This routine also cooks the
    *  output, turning "\n" into "\r\n"
    *
    *  Input parameters:
    *      str - string to write
    *
    *  Return value:
    *      number of characters written
    ********************************************************************* */

int console_xprint(const char *str)
{
	int count = 0;
	int len;
	const char *p;

	/* Convert CR to CRLF as we write things out */

	while ((p = strchr(str, '\n'))) {
		console_write((char *)str, p - str);
		console_write("\r\n", 2);
		count += (p - str);
		str = p + 1;
	}

	len = strlen(str);
	console_write((char *)str, len);
	count += len;

	return count;
}

/*  *********************************************************************
    *  console_readline_noedit(str,len)
    *
    *  A simple 'gets' type routine for the console.  We support
    *  backspace and carriage return.  No line editing support here,
    *  this routine is used in places where we don't want it.
    *
    *  Input parameters:
    *      prompt - prompt string
    *      str - pointer to user buffer
    *      len - length of user buffer
    *
    *  Return value:
    *      number of characters read (terminating newline is not
    *      placed in the buffer)
    ********************************************************************* */

int console_readline_noedit(char *prompt, char *str, int len)
{
	int reading = 1;
	char ch;
	int res = -1;
	int idx = 0;

	console_inreadline++;

	if (prompt && *prompt)
		console_write(prompt, strlen(prompt));

	POLL();
	while (reading) {

		/*
		 * If someone used console_log (above) or hit Control-C (below),
		 * redisplay the prompt and the string we've got so far.
		 */

		if (console_redisplay) {
			if (prompt && *prompt)
				console_write(prompt, strlen(prompt));
			console_write(str, idx);
			console_redisplay = 0;
			continue;
		}

		/*
		 * if nobody's typed anything, keep polling
		 */

		if (console_status() == 0) {
			POLL();
			continue;
		}

		/*
		 * Get the char from the keyboard
		 */

		res = console_read(/* arm */ (unsigned char *)&ch, 1);
		if (res < 0)
			break;
		if (res == 0)
			continue;

		/*
		 * And dispatch it
		 */

		switch (ch) {
		case 3:	/* Ctrl-C */
			console_write("^C\r\n", 4);
			console_redisplay = 1;
			idx = 0;
			break;
		case 0x7f:
		case '\b':
			if (idx > 0) {
				idx--;
				console_write("\b \b", 3);
			}
			break;
		case 21:	/* Ctrl-U */
			while (idx > 0) {
				idx--;
				console_write("\b \b", 3);
			}
			break;
		case '\r':
		case '\n':
			console_write("\r\n", 2);
			reading = 0;
			break;
		default:
			if (ch >= ' ') {
				if (idx < (len - 1)) {
					str[idx] = ch;
					idx++;
					console_write(&ch, 1);
				}
			}
			break;
		}
	}
	POLL();

	console_inreadline--;

	str[idx] = 0;
	return idx;
}

/*  *********************************************************************
    *  bolt_set_console(name)
    *
    *  This routine is usually called from the BSP's initialization
    *  module to set up the console device.  We set the xprintf
    *  callback and open the console device.  If we open a special
    *  magic console device (BOLT_BUFFER_CONSOLE) the console subsystem
    *  will buffer text instead.  A later call to bolt_set_console
    *  with a real console name will cause this text to be flushed.
    *
    *  Input parameters:
    *      name - name of console device
    *
    *  Return value:
    *      0 if ok
    *      else error code
    ********************************************************************* */

#if 0
int bolt_set_console(char *name)
{
	int res;

	xprinthook = console_xprint;

	if (!name)
		return -1;

#if !CFG_MIN_CONSOLE
	if (strcmp(name, BOLT_BUFFER_CONSOLE) == 0) {
		console_buffer_flg = 1;
		return 0;
	}
#endif

	res = env_setenv("BOOT_CONSOLE", name,
			ENV_FLG_BUILTIN | ENV_FLG_READONLY | ENV_FLG_ADMIN);
	if (res)
		return res;

	return console_open(name);
}

#if !CFG_MIN_CONSOLE

/*  *********************************************************************
    *  console_flushbuffer()
    *
    *  Flush queued (saved) console messages to the real console
    *  device.  This is used if we need to delay console
    *  initialization until after other devices are initialized,
    *  for example, with a VGA console that won't work until
    *  after PCI initialization.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

static void console_flushbuffer(void)
{
	msgqueue_t *msg;
	char *buffer;
	int length;
	int res;

	/*
	 * Remove console messages from the queue
	 */

	while ((msg = (msgqueue_t *) q_deqnext(&console_msgq))) {

		buffer = msg->data;
		length = msg->len;
		res = 0;

		/*
		 * Write each message to the console
		 */

		for (;;) {
			res =
			    bolt_write(console_handle, (unsigned char *)buffer,
				      length);
			if (res < 0)
				break;
			buffer += res;
			length -= res;
			if (length == 0)
				break;
		}

		/*
		 * Free the storage
		 */

		KFREE(msg);
	}
}

/*  *********************************************************************
    *  console_save(buffer,length)
    *
    *  This is used when we want to generate console output
    *  before the device is initialized.  The console_save
    *  routine saves a copy of the log messages in a queue
    *  for later retrieval when the console device is open.
    *
    *  Input parameters:
    *      buffer - message text to save
    *      length - number of bytes
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

static void console_save(/* arm unsigned */ char *buffer, int length)
{
	msgqueue_t *msg;

	/*
	 * Get a pointer to the last message in the queue.  If
	 * it's full, preprare to allocate a new one
	 */

	msg = (msgqueue_t *) console_msgq.q_prev;

	if (q_isempty(&(console_msgq)) || (msg->len == MSGQUEUESIZE))
		msg = NULL;

	/*
	 * Stuff characters into message chunks till we're done
	 */

	while (length) {

		/*
		 * New chunk
		 */
		if (msg == NULL) {

			msg = (msgqueue_t *) KMALLOC(sizeof(msgqueue_t), 0);
			if (msg == NULL)
				return;
			msg->len = 0;
			q_enqueue(&console_msgq, (queue_t *) msg);

			/*
			 * Remove chunks to prevent chewing too much memory
			 */

			while (q_count(&console_msgq) > MSGQUEUEMAX) {
				msgqueue_t *dropmsg;
				dropmsg =
				    (msgqueue_t *) q_deqnext(&console_msgq);
				if (dropmsg)
					KFREE(dropmsg);
			}
		}

		/*
		 * Save text.  If we run off the end of the buffer, prepare
		 * to allocate a new one
		 */
		msg->data[msg->len++] = *buffer++;
		length--;
		if (msg->len == MSGQUEUESIZE)
			msg = NULL;
	}
}

/*  *********************************************************************
    *  console_readnum(num,ch)
    *
    *  Read digits from the console until we get a non-digit.  This
    *  is used to process escape sequences like ESC [ digits ~
    *
    *  Input parameters:
    *      num - where to put returned number
    *      ch - pointer to character that starts sequence, returns
    *          character that terminated sequence
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

static void console_readnum(int *num, unsigned char *ch)
{
	int total = 0;

	for (;;) {
		total = (total * 10) + (*ch - '0');

		while (console_read(ch, 1) != 1)
			POLL();

		if (!((*ch >= '0') && (*ch <= '9')))
			break;
	}

	*num = total;
}

/*  *********************************************************************
    *  console_readkey(void)
    *
    *  Very simple ANSI escape sequence parser, returns virtual
    *  key codes for function keys, arrows, etc.
    *
    *  Hold your lunch, three levels of nested switch statements! :-)
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      virtual key code
    ********************************************************************* */

int console_readkey(void)
{
	unsigned char ch;
	int num;

	GETCHAR(ch);

	switch (ch) {
	case VKEY_ESC:
		GETCHAR(ch);
		switch (ch) {
		case 'O':
			GETCHAR(ch);
			switch (ch) {
			case 'F':
				return VKEY_END;
			case 'H':
				return VKEY_HOME;
			case 'P':
				return VKEY_F1;
			case 'Q':
				return VKEY_F2;
			case 'R':
				return VKEY_F3;
			case 'S':
				return VKEY_F4;
			}
			return (int)ch;

		case '[':
			GETCHAR(ch);
			if ((ch >= '0') && (ch <= '9')) {
				console_readnum(&num, &ch);
				if (ch == '~') {
					switch (num) {
					case 2:
						return VKEY_HOME;
					case 3:
						return VKEY_PGUP;
					case 5:
						if (console_mode == XTERM)
							return VKEY_PGUP;
						return VKEY_END;
					case 6:
						if (console_mode == XTERM)
							return VKEY_PGDN;
						return VKEY_PGDN;
					case 11:
						return VKEY_F1;
					case 12:
						return VKEY_F2;
					case 13:
						return VKEY_F3;
					case 14:
						return VKEY_F4;
					case 15:
						return VKEY_F5;
					case 17:
						return VKEY_F6;
					case 18:
						return VKEY_F7;
					case 19:
						return VKEY_F8;
					case 20:
						return VKEY_F9;
					case 21:
						return VKEY_F10;
					case 23:
						return VKEY_F11;
					case 24:
						return VKEY_F12;
					}
					return (int)ch;
				}
			} else {
				switch (ch) {
				case 'A':
					return VKEY_UP;
				case 'B':
					return VKEY_DOWN;
				case 'C':
					return VKEY_RIGHT;
				case 'D':
					return VKEY_LEFT;
				case 'F':
					return VKEY_END;
				case 'H':
					return VKEY_HOME;
				default:
					return (int)ch;
				}
			}
		default:
			return (int)ch;

		}
	default:
		return (int)ch;
	}
}

/*  *********************************************************************
    *  console_xxx(...)
    *
    *  Various small routines to write out cursor control sequences.
    *
    *  Input parameters:
    *      # of iterations, if necessary
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

static void console_backspace(int n)
{
	int t;

	for (t = 0; t < n; t++)
		console_write("\b", 1);
}

static void console_whiteout(int n)
{
	int t;

	for (t = 0; t < n; t++)
		console_write(" ", 1);
	for (t = 0; t < n; t++)
		console_write("\b", 1);
}

static void console_eraseeol(void)
{
	console_write("\033[K", 3);
}

static void console_crlf(void)
{
	console_write("\r\n", 2);
}

/*  *********************************************************************
    *  console_readline(str,len)
    *
    *  A simple 'gets' type routine for the console, with line
    *  editing support.
    *
    *  Input parameters:
    *      prompt - prompt string
    *      str - pointer to user buffer
    *      len - length of user buffer
    *
    *  Return value:
    *      number of characters read (terminating newline is not
    *      placed in the buffer)
    ********************************************************************* */

int console_readline(char *prompt, char *str, int maxlen)
{
	int reading = 1;
	int ch;
	int idx = 0;
	int len = 0;
	int t;
	int klen;
	int recall;
	int nosave = 0;
	char *x;
	char env[10];

	console_inreadline++;
	recall = console_nextsave;

	if (console_savedlines[console_nextsave]) {
		KFREE(console_savedlines[console_nextsave]);
		console_savedlines[console_nextsave] = NULL;
	}
	console_savedlines[console_nextsave] = strdup("");

	if (prompt && *prompt)
		console_write(prompt, strlen(prompt));

	POLL();
	while (reading) {

		/*
		 * If someone used console_log (above) or hit Control-C (below),
		 * redisplay the prompt and the string we've got so far.
		 */

		if (console_redisplay) {
			if (prompt && *prompt)
				console_write(prompt, strlen(prompt));
			console_write(str, idx);
			console_redisplay = 0;
			continue;
		}

		/*
		 * if nobody's typed anything, keep polling
		 */

		if (console_status() == 0) {
			POLL();
			continue;
		}

		/*
		 * Get the char from the keyboard
		 */

		ch = console_readkey();
		if (ch < 0)
			break;
		if (ch == 0)
			continue;

		/*
		 * And dispatch it.
		 Lots of yucky character manipulation follows
		 */

		switch (ch) {
		case CTRL('C'):	/* Ctrl-C - cancel line */
			console_write("^C\r\n", 4);
			console_redisplay = 1;
			nosave = 1;
			idx = 0;
			len = 0;
			break;

		case 0x7f:	/* Backspace, Delete */
		case CTRL('H'):
			if (idx > 0) {
				nosave = 0;
				len--;
				idx--;
				console_write("\b", 1);
				if (len != idx) {
					for (t = idx; t < len; t++)
						str[t] = str[t + 1];
					console_write(&str[idx], len - idx);
					console_whiteout(1);
					console_backspace(len - idx);
				} else {
					console_whiteout(1);
				}
			}
			break;

		case CTRL('W'):	 /* Kill word */
			if (idx > 0) {
				int cut;
				nosave = 0;
				/* delete any whitespace we're sitting on */
				for (t = idx - 1; t >= 0 && str[t] == ' '; t--)
					;
				/* delete the previous word */
				for (; t >= 0 && str[t] != ' '; t--)
					;
				cut = idx - t - 1;
				len -= cut;
				idx -= cut;
				console_backspace(cut);
				if (len != idx) {
					for (t = idx; t < len; t++)
						str[t] = str[t+cut];
					console_write(&str[idx], len - idx);
					console_whiteout(cut);
					console_backspace(len - idx);
				} else {
					console_whiteout(cut);
				}
			}
			break;

		case CTRL('D'):	/* Ctrl-D */
			if ((idx >= 0) && (len != idx)) {
				nosave = 0;
				len--;
				for (t = idx; t < len; t++)
					str[t] = str[t + 1];
				console_write(&str[idx], len - idx);
				console_whiteout(1);
				console_backspace(len - idx);
			}
			break;

		case CTRL('B'):	/* cursor left */
		case VKEY_LEFT:
			if (idx > 0) {
				idx--;
				console_backspace(1);
			}
			break;

		case CTRL('F'):	/* cursor right */
		case VKEY_RIGHT:
			if (idx < len) {
				console_write(&str[idx], 1);
				idx++;
			}
			break;

		case CTRL('A'):	/* cursor to BOL */
		case VKEY_HOME:
			console_backspace(idx);
			idx = 0;
			break;

		case CTRL('E'):	/* cursor to EOL */
		case VKEY_END:
			if (len - idx > 0)
				console_write(&str[idx], len - idx);
			idx = len;
			break;

		case CTRL('K'):	/* Kill to EOL */
			if (idx != len) {
				str[len] = '\0';
				if (console_killbuffer)
					KFREE(console_killbuffer);
				console_killbuffer = strdup(&str[idx]);
				console_whiteout(len - idx);
				len = idx;
				nosave = 0;
			}
			break;

		case CTRL('Y'):	/* Yank killed data */
			if (console_killbuffer == NULL)
				break;
			klen = strlen(console_killbuffer);
			if (klen == 0)
				break;
			if (len + klen > maxlen)
				break;
			nosave = 0;

			for (t = len + klen; t > idx; t--)
				str[t - 1] = str[t - klen - 1];

			for (t = 0; t < klen; t++)
				str[t + idx] = console_killbuffer[t];

			len += klen;
			console_write(&str[idx], len - idx);
			idx += klen;
			console_backspace(len - idx - 1);
			break;

		case CTRL('R'):	/* Redisplay line */
			str[len] = 0;
			console_crlf();
			if (prompt && *prompt)
				console_write(prompt, strlen(prompt));
			console_write(str, len);
			console_backspace(len - idx);
			break;

		case CTRL('U'):	/* Cancel line */
			console_backspace(idx);
			console_eraseeol();
			if (len > 0)
				nosave = 1;
			idx = 0;
			len = 0;
			break;

		case CTRL('M'):	/* terminate */
		case CTRL('J'):
			console_crlf();
			reading = 0;
			nosave = 0;
			break;

		case CTRL('P'):
		case VKEY_UP:	/* recall previous line */
			t = recall;
			t--;
			if (t < 0)
				t = MAXSAVELINES - 1;
			if (console_savedlines[t] == NULL)
				break;
			recall = t;
			console_backspace(idx);
			strcpy(str, console_savedlines[recall]);
			len = idx = strlen(console_savedlines[recall]);
			console_eraseeol();
			console_write(str, len);
			nosave = 1;
			break;

		case CTRL('N'):
		case VKEY_DOWN:	/* Recall next line */
			t = recall;
			t++;
			if (t >= MAXSAVELINES)
				t = 0;
			if (console_savedlines[t] == NULL)
				break;
			recall = t;
			console_backspace(idx);
			strcpy(str, console_savedlines[recall]);
			len = idx = strlen(console_savedlines[recall]);
			console_eraseeol();
			console_write(str, len);
			nosave = 1;
			break;

		case VKEY_F1:
		case VKEY_F2:
		case VKEY_F3:
		case VKEY_F4:
		case VKEY_F5:
		case VKEY_F6:
		case VKEY_F7:
		case VKEY_F8:
		case VKEY_F9:
		case VKEY_F10:
		case VKEY_F11:
		case VKEY_F12:
			sprintf(env, "F%d", ch - VKEY_F1 + 1);
			x = env_getenv(env);
			if (x) {
				console_backspace(idx);
				strcpy(str, x);
				idx = len = strlen(str);
				console_eraseeol();
				console_write(str, len);
				console_crlf();
				reading = 0;
				nosave = 0;
			}
			/*
			 * If F12 is undefined, it means "repeat last command"
			 */
			if (ch == VKEY_F12) {
				t = recall;
				t--;
				if (t < 0)
					t = MAXSAVELINES - 1;
				if (console_savedlines[t] == NULL)
					break;
				recall = t;
				console_backspace(idx);
				strcpy(str, console_savedlines[recall]);
				len = idx = strlen(console_savedlines[recall]);
				console_eraseeol();
				console_write(str, len);
				console_crlf();
				reading = 0;
				nosave = 1;
			}
			break;

		case VKEY_PGUP:
		case VKEY_PGDN:
			/*
			 * These control sequences aren't very consistent, and
			 * we don't need to scroll through pages of command
			 * history anway, as we don't keep very many in memory
			 */
			break;

		default:	/* insert character */
			if (ch >= ' ') {
				if (idx < (maxlen - 1)) {
					nosave = 0;

					for (t = len; t > idx; t--)
						str[t] = str[t - 1];

					str[idx] = ch;
					len++;
					if (len != idx) {
						console_write(&str[idx],
							      len - idx);
						console_backspace(len - idx -
								  1);
					}
					idx++;
				}
			}
			break;
		}
	}
	POLL();

	console_inreadline--;

	str[len] = 0;

	if ((len != 0) && !nosave) {

		if (console_savedlines[console_nextsave])
			KFREE(console_savedlines[console_nextsave]);

		console_savedlines[console_nextsave] = strdup(str);
		console_nextsave++;

		if (console_nextsave == MAXSAVELINES)
			console_nextsave = 0;
	}

	return len;
}
#endif

#else /* CFG_MIN_CONSOLE */
/*  *********************************************************************
    *  console_readline(str,len)
    *
    *  A simple 'gets' type routine for the console,
    *  just calls the "noedit" variant for minimal code
    *  size builds.
    *
    *  Input parameters:
    *      prompt - prompt string
    *      str - pointer to user buffer
    *      len - length of user buffer
    *
    *  Return value:
    *      number of characters read (terminating newline is not
    *      placed in the buffer)
    ********************************************************************* */

int console_readline(char *prompt, char *str, int len)
{
	return console_readline_noedit(prompt, str, len);
}

#endif
