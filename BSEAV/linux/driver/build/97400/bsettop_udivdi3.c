/***************************************************************************
 *     Copyright (c) 2004-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************/
#include "bstd.h"



#define CHAR_BIT 8
#define LONG_BITS   (sizeof(long) * CHAR_BIT)
#define HALF_BITS   (sizeof(long) * CHAR_BIT / 2)

#define B   (1 << HALF_BITS)    /* digit base */

/* Combine two `digits' to make a single two-digit number. */
#define COMBINE(a, b) (((u_long)(a) << HALF_BITS) | (b))


/* select a type for digits in base B: use unsigned short if they fit */
#if ULONG_MAX == 0xffffffff && USHRT_MAX >= 0xffff
typedef unsigned short digit;
#else
typedef uint32_t digit;
#endif

/*
 * Depending on the desired operation, we view a `long long' (aka quad_t) in
 * one or more of the following formats.
 */
union uu {
    uint64_t  q;      /* as a (signed) quad */
    uint64_t  uq;     /* as an unsigned quad */
    int32_t   sl[2];      /* as two signed longs */
    uint32_t  ul[2];      /* as two unsigned longs */
};


#define H       1
#define L       0

#define HHALF(x)    ((x) >> HALF_BITS)
#define LHALF(x)    ((x) & ((1 << HALF_BITS) - 1))
#define LHUP(x)     ((x) << HALF_BITS)


static void udivide64(uint64_t n, uint64_t d, uint64_t *pResult, uint64_t *pRemainder)
{
	uint64_t r = 0;
        uint64_t rem = 0;

	if (n == 0) {
	/* If n == 0, special case to 0 (0/x == 0) */
		r = 0;
                rem = 0;
	} else if (d > n) {
	/* If d > n, special case to 0 (n would be the remainder) */
		r = 0;
                rem = n;
	} else if (n < (d*2)) {
	/* If n < d*2, special case to 1 */
		r = 1;
                rem = n-d;
	} else if ((d <= (uint64_t)0x00000000FFFFFFFF) && (n <= (uint64_t)0x00000000FFFFFFFF)) {
	/* If they are 32bit quantities, go ahead and perform a basic 32bit div instead */
		uint32_t n32 = (uint32_t)n;
		uint32_t d32 = (uint32_t)d;
		uint32_t r32 = n32/d32;

		r = r32;

                r32 = n32%d32;
                rem = r32;
	} else { /* Otherwise, complicated division time */

		/* This segment of code is based on the routine by Ian Kaplan at 
		 * http://www.bearcave.com/software/divide.htm, which was licensed at 
		 * the time as:
		 *
		 * "Use of this program, for any purpose, is granted the author, Ian 
		 * Kaplan, as long as this copyright notice is included in the source code 
		 * or any source code derived from this program.  The user assumes all
		 * responsibility for using this code.
		 *
		 * Ian Kaplan, October 1996
		 */
		uint64_t t = 0, tmp = 0;
		uint32_t bits = 64;
		uint32_t ix;

		r = 0;
		/* Figure out about what the order of the remainder would be, to shortcut 
		 * a great deal of the math */
		while (rem < d) {
			rem = (rem << 1) | (n>>63);
			tmp = n;
			n = n << 1;
			bits--;
		}
		/* Undo the last step, since we just went one too far */
		n = tmp;
		rem = rem >> 1;
		bits++;
		/* And now, buckle down and do the rest of the work */
		for (ix=0; ix < bits; ix++) {
			rem = (rem << 1) | (n>>63);
			tmp = rem - d;
			t = !(tmp>>63);
			n = n << 1;
			r = (r << 1) | t;
			if (t) {
				rem = tmp;
			}
		}
		/* End of code based on Ian Kaplan's work. */
	}

    *pResult = r;
    *pRemainder = rem;
    return;
}

/* This is a custom re-implementation of udivdi3 (a function call emitted by gcc to handle 
 * long long integer division.  The basic algorithm handles certain easy cases first, then 
 * goes into binary long division
 */
uint64_t __udivdi3 (uint64_t n, uint64_t d) /* [n]umerator, [d]enominator */
{
   uint64_t result, unused;

   udivide64(n,d,&result, &unused);
   return result;
}

uint64_t __umoddi3 (uint64_t n, uint64_t d) /* [n]umerator, [d]enominator */
{
   uint64_t remainder, unused;

   udivide64(n,d,&unused,&remainder);
   return remainder;
}


/*
 * Shift p[0]..p[len] left `sh' bits, ignoring any bits that
 * `fall out' the left (there never will be any such anyway).
 * We may assume len >= 0.  NOTE THAT THIS WRITES len+1 DIGITS.
 */
static void
shl(register digit *p, register int len, register int sh)
{
    register int i;

    for (i = 0; i < len; i++)
        p[i] = LHALF(p[i] << sh) | (p[i + 1] >> (HALF_BITS - sh));
    p[i] = LHALF(p[i] << sh);
}



/*
 * __qdivrem(u, v, rem) returns u/v and, optionally, sets *rem to u%v.
 *
 * We do this in base 2-sup-HALF_BITS, so that all intermediate products
 * fit within u_long.  As a consequence, the maximum length dividend and
 * divisor are 4 `digits' in this base (they are shorter if they have
 * leading zeros).
 */
uint64_t
__qdivrem(uq, vq, arq)
    uint64_t uq, vq, *arq;
{
    union uu tmp;
    digit *u, *v, *q;
    register digit v1, v2;
    u_long qhat, rhat, t;
    int m, n, d, j, i;
    digit uspace[5], vspace[5], qspace[5];

    /*
     * Take care of special cases: divide by zero, and u < v.
     */
    if (vq == 0) {
        /* divide by zero. */
        static volatile const unsigned int zero = 0;

        tmp.ul[H] = tmp.ul[L] = 1 / zero;
        if (arq)
            *arq = uq;
        return (tmp.q);
    }
    if (uq < vq) {
        if (arq)
            *arq = uq;
        return (0);
    }
    u = &uspace[0];
    v = &vspace[0];
    q = &qspace[0];

    /*
     * Break dividend and divisor into digits in base B, then
     * count leading zeros to determine m and n.  When done, we
     * will have:
     *  u = (u[1]u[2]...u[m+n]) sub B
     *  v = (v[1]v[2]...v[n]) sub B
     *  v[1] != 0
     *  1 < n <= 4 (if n = 1, we use a different division algorithm)
     *  m >= 0 (otherwise u < v, which we already checked)
     *  m + n = 4
     * and thus
     *  m = 4 - n <= 2
     */
    tmp.uq = uq;
    u[0] = 0;
    u[1] = HHALF(tmp.ul[H]);
    u[2] = LHALF(tmp.ul[H]);
    u[3] = HHALF(tmp.ul[L]);
    u[4] = LHALF(tmp.ul[L]);
    tmp.uq = vq;
    v[1] = HHALF(tmp.ul[H]);
    v[2] = LHALF(tmp.ul[H]);
    v[3] = HHALF(tmp.ul[L]);
    v[4] = LHALF(tmp.ul[L]);
    for (n = 4; v[1] == 0; v++) {
        if (--n == 1) {
            u_long rbj; /* r*B+u[j] (not root boy jim) */
            digit q1, q2, q3, q4;

            /*
             * Change of plan, per exercise 16.
             *  r = 0;
             *  for j = 1..4:
             *      q[j] = floor((r*B + u[j]) / v),
             *      r = (r*B + u[j]) % v;
             * We unroll this completely here.
             */
            t = v[2];   /* nonzero, by definition */
            q1 = u[1] / t;
            rbj = COMBINE(u[1] % t, u[2]);
            q2 = rbj / t;
            rbj = COMBINE(rbj % t, u[3]);
            q3 = rbj / t;
            rbj = COMBINE(rbj % t, u[4]);
            q4 = rbj / t;
            if (arq)
                *arq = rbj % t;
            tmp.ul[H] = COMBINE(q1, q2);
            tmp.ul[L] = COMBINE(q3, q4);
            return (tmp.q);
        }
    }

    /*
     * By adjusting q once we determine m, we can guarantee that
     * there is a complete four-digit quotient at &qspace[1] when
     * we finally stop.
     */
    for (m = 4 - n; u[1] == 0; u++)
        m--;
    for (i = 4 - m; --i >= 0;)
        q[i] = 0;
    q += 4 - m;

    /*
     * Here we run Program D, translated from MIX to C and acquiring
     * a few minor changes.
     *
     * D1: choose multiplier 1 << d to ensure v[1] >= B/2.
     */
    d = 0;
    for (t = v[1]; t < B / 2; t <<= 1)
        d++;
    if (d > 0) {
        shl(&u[0], m + n, d);       /* u <<= d */
        shl(&v[1], n - 1, d);       /* v <<= d */
    }
    /*
     * D2: j = 0.
     */
    j = 0;
    v1 = v[1];  /* for D3 -- note that v[1..n] are constant */
    v2 = v[2];  /* for D3 */
    do {
        register digit uj0, uj1, uj2;

        /*
         * D3: Calculate qhat (\^q, in TeX notation).
         * Let qhat = min((u[j]*B + u[j+1])/v[1], B-1), and
         * let rhat = (u[j]*B + u[j+1]) mod v[1].
         * While rhat < B and v[2]*qhat > rhat*B+u[j+2],
         * decrement qhat and increase rhat correspondingly.
         * Note that if rhat >= B, v[2]*qhat < rhat*B.
         */
        uj0 = u[j + 0]; /* for D3 only -- note that u[j+...] change */
        uj1 = u[j + 1]; /* for D3 only */
        uj2 = u[j + 2]; /* for D3 only */
        if (uj0 == v1) {
            qhat = B;
            rhat = uj1;
            goto qhat_too_big;
        } else {
            u_long nn = COMBINE(uj0, uj1);
            qhat = nn / v1;
            rhat = nn % v1;
        }
        while (v2 * qhat > COMBINE(rhat, uj2)) {
    qhat_too_big:
            qhat--;
            if ((rhat += v1) >= B)
                break;
        }
        /*
         * D4: Multiply and subtract.
         * The variable `t' holds any borrows across the loop.
         * We split this up so that we do not require v[0] = 0,
         * and to eliminate a final special case.
         */
        for (t = 0, i = n; i > 0; i--) {
            t = u[i + j] - v[i] * qhat - t;
            u[i + j] = LHALF(t);
            t = (B - HHALF(t)) & (B - 1);
        }
        t = u[j] - t;
        u[j] = LHALF(t);
        /*
         * D5: test remainder.
         * There is a borrow if and only if HHALF(t) is nonzero;
         * in that (rare) case, qhat was too large (by exactly 1).
         * Fix it by adding v[1..n] to u[j..j+n].
         */
        if (HHALF(t)) {
            qhat--;
            for (t = 0, i = n; i > 0; i--) { /* D6: add back. */
                t += u[i + j] + v[i];
                u[i + j] = LHALF(t);
                t = HHALF(t);
            }
            u[j] = LHALF(u[j] + t);
        }
        q[j] = qhat;
    } while (++j <= m);     /* D7: loop on j. */

    /*
     * If caller wants the remainder, we have to calculate it as
     * u[m..m+n] >> d (this is at most n digits and thus fits in
     * u[m+1..m+n], but we may need more source digits).
     */
    if (arq) {
        if (d) {
            for (i = m + n; i > m; --i)
                u[i] = (u[i] >> d) |
                    LHALF(u[i - 1] << (HALF_BITS - d));
            u[i] = 0;
        }
        tmp.ul[H] = COMBINE(uspace[1], uspace[2]);
        tmp.ul[L] = COMBINE(uspace[3], uspace[4]);
        *arq = tmp.q;
    }

    tmp.ul[H] = COMBINE(qspace[1], qspace[2]);
    tmp.ul[L] = COMBINE(qspace[3], qspace[4]);
    return (tmp.q);
}


#if 1
/*
* Custom implementation; DTV does require it
*/
int64_t __divdi3(int64_t a, int64_t b)
{
    int64_t ua, ub, uq;
    int neg = 0;

    ua = a;
    ub = b;

    if (a < 0)
        ua = -ua, neg ^= 1;
    if (b < 0)
        ub = -ub, neg ^= 1;

    uq = __qdivrem((uint64_t)ua, (uint64_t)ub, (uint64_t *)0);

    if (neg)
        uq = - uq;
    return uq;
}
#endif



