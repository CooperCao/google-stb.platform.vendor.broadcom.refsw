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
/* $Id: termio.c,v 1.6 1998/03/30 15:12:53 chris Exp $ */
#include "queue.h"
#include "stdio.h"
#include "termio.h"
/*#include "mips.h"*/

extern ConfigEntry ConfigTable[];
extern char *getenv(const char *);
extern long gsignal (char *jb, int sig);
extern void sbdpoll (void);
extern int getbaudrate (char *p);
extern char *strcpy(char *dest, const char *src);

void scandevs(void);
void _chwrite (DevEntry *p, char ch);
int write (int fd, char *buf, int n);
int _write (int fd, char *buf, int n);

DevEntry DevTable[DEV_MAX];

File _file[OPEN_MAX] =
{
    {0, 1},
    {0, 1},
    {0, 1}
};

int _pmon_in_ram = 0;

void reschedule (void)
{
    scandevs ();
}

/*static char DbgString[96];*/

/** _write(fd,buf,n) write n bytes from buf to fd */
int _write (fd, buf, n)
     int             fd, n;
     char           *buf;
{
    int             i;
    DevEntry       *p;

    if (!_file[fd].valid)
        return (-1);

/*
//_writeasm('X');
//   sprintf(DbgString, "*** fd=%x, dev=%x, n=%d\n", fd, _file[fd].dev, n);
//   dbg_print(DbgString);
*/

    p = &DevTable[_file[fd].dev];

    for (i = 0; i < n; i++)
    {
		if (p->t.c_oflag & ONLCR && buf[i] == '\n') 
		{
	    	_chwrite (p, '\r');
		}

		_chwrite (p, buf[i]);
    }

    return (i);
}

void _chwrite (DevEntry *p, char ch)
{
    while (p->txoff)
		scandevs(); 
	
    if (p->handler)
    {
		while (!(*p->handler) (OP_TXRDY, p->sio, p->chan))
	  		scandevs ();
  		
		(*p->handler) (OP_TX, p->sio, p->chan, ch);
    } 
}

void scandevs (void)
{
    int             c, n;
    DevEntry       *p;

    for (p = DevTable; p->rxq; p++) {
	while ((*p->handler) (OP_RXRDY, p->sio, p->chan)) {
	    c = (*p->handler) (OP_RX, p->sio, p->chan);
	    if (p->t.c_iflag & ISTRIP)
		c &= 0x7f;
	    if (p->t.c_lflag & ISIG) {
		if (c == p->t.c_cc[VINTR]) {
		    gsignal ((char *)(p->intr), 2 /*SIGINT*/);
		    continue;
		}
	    }
	    if (p->t.c_iflag & IXON) {
		if (p->t.c_iflag & IXANY && p->txoff) {
		    p->txoff = 0;
		    continue;
		}
		if (c == p->t.c_cc[V_STOP]) {
		    p->txoff = 1;
		    continue;
		}
		if (c == p->t.c_cc[V_START]) {
		    p->txoff = 0;
		    continue;
		}
	    }

	    n = Qspace (p->rxq);
	    if (n > 0) {
		Qput (p->rxq, c);
		if (n < 20 && !p->rxoff) {
		    (*p->handler) (OP_RXSTOP, p->sio, p->chan, p->rxoff = 1);
		    if (p->t.c_iflag & IXOFF)
			_chwrite (p, CNTRL ('S'));
		}
	    }
	}
    }

    sbdpoll();
}


int devinit (void)
{
    int             i, brate;
    ConfigEntry    *q;
    DevEntry       *p;
    char	   dname[5];
    char	   *s;

    strcpy (dname, "ttyx");
    for (i = 0; ConfigTable[i].devinfo && i < DEV_MAX; i++) {
	q = &ConfigTable[i];
	p = &DevTable[i];
	p->txoff = 0;
	p->rxoff = 0;
	if (q->chan == 0 && !_pmon_in_ram)
	    (*q->handler) (OP_INIT, q->devinfo, 0, q->rxqsize);
	p->qsize = q->rxqsize;
	p->rxq = Qcreate (p->qsize);

	if (p->rxq == 0)
	    return (-1);

	dname[3] = (i < 10) ? i + '0' : i - 10 + 'a';

	if (!(s = getenv (dname)) || (brate = getbaudrate (s)) == 0)
	    brate = q->brate;
	if (!_pmon_in_ram) {

	    /* set default baudrate in case anything goes wrong */
	    if (q->brate & CXBAUD)
		(void)(*q->handler) (OP_XBAUD, q->devinfo, q->chan, q->brate);
	    else
		(void) (*q->handler) (OP_BAUD, q->devinfo, q->chan, q->brate);

	    /*
	     * program requested baud rate, but fall back to default
	     * if there is a problem
	     */
	    if (brate != q->brate) {
			if (brate & CXBAUD) {
		    	if ((*p->handler) (OP_XBAUD, q->devinfo, q->chan, brate))
					brate = q->brate;
			}
			else {
		    	if ((*q->handler) (OP_BAUD, q->devinfo, q->chan, brate))
					brate = q->brate;
			}
	    }
	}
   
	p->sio = q->devinfo;
	p->chan = q->chan;
	p->handler = q->handler;
	p->intr = 0;
	p->tfunc = 0;
	p->t.c_iflag = (ISTRIP | ICRNL | IXON);
	p->t.c_oflag = (ONLCR);
	p->t.c_lflag = (ICANON | ISIG | ECHO | ECHOE);
	p->t.c_cflag = brate;
	p->t.c_cc[VINTR] = CNTRL ('c');
	p->t.c_cc[VEOL] = '\n';
	p->t.c_cc[VEOL2] = CNTRL ('c');
	p->t.c_cc[VERASE] = CNTRL ('h');
	p->t.c_cc[V_STOP] = CNTRL ('s');
	p->t.c_cc[V_START] = CNTRL ('q');
	if (!_pmon_in_ram)
	    _chwrite (p, CNTRL ('Q'));
    }
    return (0);
}

/** ttctl(fd,op,a1,a2) perform terminal specific operation */
int ttctl (int fd, int op, int a1, int a2)
{
    DevEntry       *p;
    int             r;

    if (!_file[fd].valid || _file[fd].dev < 0)
        return (-1);

    p = &DevTable[_file[fd].dev];
    if (p->tfunc == 0)
	return (-1);
    r = (*p->tfunc) (fd, op, a1, a2);
    return (r);
}


/** _read(fd,buf,n) read n bytes into buf from fd */
int _read (int fd, char *buf, int n)
{
    int             i, used;
    DevEntry       *p;
    char            ch;

    if (!_file[fd].valid)
        return (-1);

    p = &DevTable[_file[fd].dev];
    for (i = 0; i < n;) {
		scandevs ();
		while ((used = Qused (p->rxq)) == 0)
	    	reschedule ();
	    
		if (used < 20 && p->rxoff) {
	   		(*p->handler) (OP_RXSTOP, p->sio, p->chan, p->rxoff = 0);
	    	if (p->t.c_iflag & IXOFF)
				_chwrite (p, CNTRL ('Q'));
		}

		ch = Qget (p->rxq);
		if (p->t.c_iflag & ICRNL && ch == '\r')
	    	ch = '\n';
		if (p->t.c_lflag & ICANON) {
	    	buf[i] = ch;
#if 0
        	if (ch == p->t.c_cc[VERASE]) {
				if (i > 0) {
		    		i--;
		    		if (p->t.c_lflag & ECHOE)
		      			write (fd, "\b ", 2);
		    		else if (p->t.c_lflag & ECHO)
		      			write (fd, "\b", 1);
				}
				else {
              		continue;
				}
	    	}
        	else
#endif
          		i++;

	    	if (p->t.c_lflag & ECHO)
	        	write (fd, &ch, 1);

#if 0
        	if (ch == p->t.c_cc[VEOL] || ch == p->t.c_cc[VEOL2])
	      		break;
#endif
			break;
		} else { /* not ICANON */
	    	buf[i++] = ch;
		}
		
    } /* end of for loop */
    return (i);
}

int read (int fd, char *buf, int n)
{
	return (_read (fd, buf, n));
}

int write (int fd, char *buf, int n)
{
	return (_write (fd, buf, n));
}


void ioctl(void) {}
void lseek(void) {}
void close(void) {}
