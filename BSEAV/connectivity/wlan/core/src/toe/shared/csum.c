/*
 * Broadcom internal C reference implementation of lwIP-specific IP checksum function
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

uint16
arm_inet_checksum(const uint8 *pb, int len)
{
	uint16 *ps, t = 0;
	uint32 *pl;
	uint32 sum = 0, tmp;
	int odd = ((uint32)pb & 1);	/* starts at odd byte address? */

	if (odd && len > 0) {
		((uint8 *)&t)[1] = *pb++;
		len--;
	}

	ps = (uint16 *)pb;

	if (((uint32)ps & 3) && len > 1) {
		sum += *ps++;
		len -= 2;
	}

	pl = (uint32 *)ps;

	while (len & ~15)  {
		tmp = sum + *pl++;	/* ping */
		if (tmp < sum)
			tmp++;		/* add back carry */

		sum = tmp + *pl++;	/* pong */
		if (sum < tmp)
			sum++;		/* add back carry */

		tmp = sum + *pl++;	/* ping */
		if (tmp < sum)
			tmp++;		/* add back carry */

		sum = tmp + *pl++;	/* pong */
		if (sum < tmp)
			sum++;		/* add back carry */

		len -= 16;
	}

	/* fold to make room in upper bits */
	sum = (sum >> 16) + (sum & 0xffff);

	ps = (uint16 *)pl;

	/* zero to seven 16-bit aligned shorts remaining */
	while (len > 1) {
		sum += *ps++;
		len -= 2;
	}

	/* dangling tail byte remaining? */
	if (len > 0)			/* include odd byte */
		((uint8 *)&t)[0] = *(uint8 *)ps;

	sum += t;			/* add end bytes */

	/* fold twice */
	sum = (sum >> 16) + (sum & 0xffff);
	sum = (sum >> 16) + (sum & 0xffff);

	/* byte-swap sum if original alignment was odd */
	if (odd)
		sum = (sum << 8 | sum >> 8) & 0xffff;

	return sum;
}
