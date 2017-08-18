/*
* Customer HW 10 dependant file
* Copyright (C) 2017, Broadcom. All Rights Reserved.
* 
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
* 
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
* OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
* CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
* $Id$
*/

#ifdef SOFTAP_TPUT_ENHANCE
extern int set_softap_params(dhd_pub_t *dhd);
#endif /* SOFTAP_TPUT_ENHANCE */

#if defined(DHD_TCP_WINSIZE_ADJUST)
extern int dhd_adjust_tcp_winsize(int index, int pk_type, int op_mode, struct sk_buff *skb);
#endif /* DHD_TCP_WINSIZE_ADJUST */
