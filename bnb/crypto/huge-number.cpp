/*
** huge.c
**
** Arbitrary precision integer library from Python sources.
**
** This is a minor modification of the file "huge-number.c" taken from
** mirrordir-0.10.49 which in turn contains these copyrights ...
**
** $Id: huge.c,v 1.1.1.1 2001/04/12 18:08:01 ndwinton Exp $
*/

/* huge-number.c: arbitrary precision integer library from Python sources
   This has nothing to do with cryptography.
   Copyright (C) 1998 Paul Sheer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* This file was taken from the Python source for `long' type
   integers. I have changed it to compile independently of the
   Python source, and added the optimisation that GNU C can
   use 31 bit digits instead of Python's 15 bit. You can download
   the original from www.python.org. This file bears little
   resemblance to the original though - paul */

/***********************************************************
Copyright 1991-1995 by Stichting Mathematisch Centrum, Amsterdam,
The Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI or Corporation for National Research Initiatives or
CNRI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

While CWI is the initial source for this software, a modified version
is made available by the Corporation for National Research Initiatives
(CNRI) at the Internet address ftp://ftp.python.org.

STICHTING MATHEMATISCH CENTRUM AND CNRI DISCLAIM ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH
CENTRUM OR CNRI BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "huge-number.h"

using namespace huge;

#undef ABS
#undef MAX
#undef MIN
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#ifdef __GNUC__
#define huge_assert(x) { if (!(x)) { fprintf (stderr, "huge: assertion failed, %s:%d\n", __FILE__, __LINE__); abort(); } }
#else
#define huge_assert(x) { if (!(x)) abort(); }
#endif

#define huge_error(x) fprintf (stderr, "huge_%s\n", x)

#define num_size(x) (((number*)(x))->m_size)
#define num_digit(x) (((number*)(x))->m_digit)

/* Normalize (remove leading zeros from) a long int object.
Doesn't attempt to free the storage--in most cases, due to the nature
of the algorithms used, this could save at most be one word anyway. */

number* number::huge_normalize(number* v)
{
	digsize j = ABS(num_size(v));
	digsize i = j;

	while (i > 0 && num_digit(v)[i - 1] == 0)
		--i;
	if (i != j)
		num_size(v) = (num_size(v) < 0) ? -(i) : i;
	return v;
}

number* number::make(digsize size)
{
	number* h;
	h =(number*)malloc(sizeof(digsize) + ((size_t)ABS(size)) * sizeof(digit));
	num_size(h) = size;
	memset(num_digit(h), 0, ABS(size) * sizeof(digit));
	return h;
}

void number::copy(number* a, number* b)
{
	digsize i;
	for (i = 0; i < ABS(num_size(b)); i++)
		num_digit(a)[i] = num_digit(b)[i];
	num_size(a) = num_size(b);
}

number* number::dup(number* a)
{
	number* b;
	if (!a)
		return 0;
	b = make(ABS(num_size(a)));
	copy(b, a);
	return b;
}

/* we want to wipe as we go along, so that secret keys cannot be read from memory: */
void number::erase(number*& a)
{
	if (a) {
		memset(a, 0, sizeof(digsize) + ((size_t)ABS(num_size(a)) * sizeof(digit)));
		free(a);
	}
	a = 0;
}

/* Create a new long int object from a C long int */

number* number::from_long(long ival)
{
	/* Assume a C long fits in at most 5 'digits' */
	/* Works on both 32- and 64-bit machines */
	number* v = make(5);
	unsigned long t = ival;
	digsize i;
	if (ival < 0) {
		t = -ival;
		num_size(v) = -(num_size(v));
	}
	for (i = 0; i < 5; i++) {
		num_digit(v)[i] = (digit)(t & MASK);
		t >>= SHIFT;
	}
	return huge_normalize(v);
}

number* number::set_bit(number* v, digsize i)
{
	number* w;
	w = make(MAX(ABS(num_size(v)), i / SHIFT + 1));
	copy(w, v);
	num_digit(w)[i / SHIFT] |= (1 << (i % SHIFT));
	return w;
}

void number::clear_bit(number* v, digsize i)
{
	if (i / SHIFT < ABS(num_size(v)))
		num_digit(v)[i / SHIFT] &= ~(1 << (i % SHIFT));
	huge_normalize(v);
}

unsigned char number::_huge_get_char(number* a, digsize j)
{
	twodigits r = 0;
	digsize i;
	i = j * 8 / SHIFT;
	if (i < num_size(a)) {
		r = num_digit(a)[i];
		if (++i < num_size(a))
			r |= (twodigits)num_digit(a)[i] << SHIFT;
	}
	r >>= ((j * 8) % SHIFT);
	return (unsigned char)(r & 0xFF);
}

/* result must be free'd */
char* number::as_binary(number* a, digsize *l)
{
	char *s;
	digsize i;
	*l = (num_size(a) * SHIFT) / 8 + 1;
	s = (char*)malloc(*l + 1);
	for (i = 0; i < *l; i++)
		s[i] = _huge_get_char(a, i);
	while (*l > 0 && !s[*l - 1])
		(*l)--;
	return s;
}

/* result must be free'd */
number* number::from_binary(unsigned char *s, digsize n)
{
	number* z;
	digsize i, size;
	digit *d;
	size = n * 8 / SHIFT;
	z = make(size + 1);
	d = num_digit(z);
	for (i = 0; i < size + 1; i++) {
		digsize j;
		twodigits t = 0;
		digsize r;
		r = i * SHIFT / 8;
		for (j = 0; j < SHIFT / 8 + 3 && r < n; j++, r++)
			t |= (twodigits)s[r] << (j * 8);
		t >>= ((i * SHIFT) % 8);
		*d++ = (digit)t & MASK;
	}
	return huge_normalize(z);
}

/* Create a new long int object from a C unsigned long int */

number* number::from_unsigned_long(unsigned long ival)
{
	unsigned long t = ival;
	digsize i;
	/* Assume a C long fits in at most 5 'digits' */
	/* Works on both 32- and 64-bit machines */
	number* v = make(5);
	for (i = 0; i < 5; i++) {
		num_digit(v)[i] = (digit)(t & MASK);
		t >>= SHIFT;
	}
	return huge_normalize(v);
}

/* Get a C long int from a long int object.
Returns -1 and sets an error condition if overflow occurs. */

long number::as_long(number* v)
{
	long x, prev;
	digsize i;
	int sign;

	i = num_size(v);
	sign = 1;
	x = 0;
	if (i < 0) {
		sign = -1;
		i = -(i);
	}
	while (--i >= 0) {
		prev = x;
		x = (x << SHIFT) + num_digit(v)[i];
		if ((x >> SHIFT) != prev) {
			huge_error("as_long(): long int too long to convert");
			return -1;
		}
	}
	return x * sign;
}

/* Get a C long int from a long int object.
Returns -1 and sets an error condition if overflow occurs. */

unsigned long number::as_unsigned_long(number* v)
{
	unsigned long x, prev;
	digsize i;

	i = num_size(v);
	x = 0;
	if (i < 0) {
		huge_error("as_unsigned_long(): can't convert negative value to unsigned long");
		return (unsigned long) -1;
	}
	while (--i >= 0) {
		prev = x;
		x = (x << SHIFT) + num_digit(v)[i];
		if ((x >> SHIFT) != prev) {
			huge_error("as_unsigned_long(): long int too long to convert");
			return (unsigned long) -1;
		}
	}
	return x;
}

/*
*    gcc knows about 64bit product, so no optimisation needed:
*
*      pushl -8(%ebp)
*      pushl $.LC2
*      call printf
*.stabn 68,0,47,.LM64-from_long
*.LM64:
*      pushl %edi
*      pushl $.LC2
*      call printf
*.stabn 68,0,48,.LM65-from_long
*.LM65:
*      movl -8(%ebp),%eax
*      imull %edi
*      movl %eax,-16(%ebp)
*      movl %edx,-12(%ebp)
*.stabn 68,0,49,.LM66-from_long
*.LM66:
*      pushl -12(%ebp)
*      pushl -16(%ebp)
*      pushl $.LC2
*      call printf
*/

number* number::muladd1(number* a, wdigit n, wdigit extra)
{
	digsize size_a = ABS(num_size(a));
	number* z = make(size_a + 1);
	twodigits carry = extra;
	digsize i;
	for (i = 0; i < size_a; ++i) {
		carry += (twodigits)num_digit(a)[i] * n;
		num_digit(z)[i] = (digit)(carry & MASK);
		carry >>= SHIFT;
	}
	num_digit(z)[i] = (digit)carry;
	return huge_normalize(z);
}

/* Divide a long integer by a digit, returning both the quotient
(as function result) and the remainder (through *prem).
The sign of a is ignored; n should not be zero. */

number* number::divrem1(number* a, wdigit n, digit * prem)
{
	digsize size = ABS(num_size(a));
	number* z;
	digsize i;
	twodigits rem = 0;

	huge_assert(n > 0 && n <= MASK);
	z = make(size);
	for (i = size; --i >= 0;) {
		rem = (rem << SHIFT) + num_digit(a)[i];
		num_digit(z)[i] = (digit)(rem / n);
		rem %= n;
	}
	*prem = (digit)rem;
	return huge_normalize(z);
}

/* Convert a long int object to a string, using a given conversion base.
Return a string object.

NDW: The following does not apply here ....
If base is 8 or 16, add the proper prefix '0' or '0x'.
External linkage: used in bltinmodule.c by hex() and oct(). */

char* number::format(number* a, int base)
{
	char *str;
	int i;
	int size_a = ABS(num_size(a));
	char *p;
	int bits;
	char sign = '\0';

	a = dup(a);
	huge_assert(base >= 2 && base <= 36);

	/* Compute a rough upper bound for the length of the string */
	i = base;
	bits = 0;
	while (i > 1) {
		++bits;
		i >>= 1;
	}
	i = 6 + (size_a * SHIFT + bits - 1) / bits;
	str = (char*)malloc(i + 1);
	p = str + i;
	*p = '\0';
#ifdef ORIGINAL_BEHAVIOUR
	*--p = 'L';
#endif
	if (num_size(a) < 0) {
		sign = '-';
		num_size(a) = ABS(num_size(a));
	}

	do {
		digit rem;
		number* temp = divrem1(a, (digit)base, &rem);
		if (temp == 0) {
			erase(a);
			free (str);
			return 0;
		}
		if (rem < 10)
			rem += '0';
		else
			rem += 'a' - 10;
		huge_assert(p > str);
		*--p = (char) rem;
		erase(a);
		a = temp;
	} while (ABS(num_size(a)) != 0);
	erase(a);
#ifdef ORIGINAL_BEHAVIOUR
	/* NDW -- removed this for GMP compatibility */
	if (base == 8) {
		if (size_a != 0)
			*--p = '0';
	} else if (base == 16) {
		*--p = 'x';
		*--p = '0';
	} else if (base != 10) {
		*--p = '#';
		*--p = '0' + base % 10;
		if (base > 10)
			*--p = '0' + base / 10;
	}
#endif
	if (sign)
		*--p = sign;
	if (p != str) {
		char *q = str;
		huge_assert(p > q);
		do {
		} while ((*q++ = *p++) != '\0');
		q--;
	}
	return str;
}

number* number::from_string(char *str, char **pend, int base)
{
	int sign = 1;
	number* z;

	while (*str != '\0' && strchr ("\t\n ", *str))
		str++;
	if (*str == '+')
		++str;
	else if (*str == '-') {
		++str;
		sign = -1;
	}
	while (*str != '\0' && strchr ("\t\n ", *str))
		str++;
	if (base == 0) {
		if (str[0] != '0')
			base = 10;
		else if (str[1] == 'x' || str[1] == 'X')
			base = 16;
		else
			base = 8;
	}
	if (base == 16 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		str += 2;
	z = make(0);
	for (; z != 0; ++str) {
		int k = -1;
		number* temp;

		if (*str <= '9')
			k = *str - '0';
		else if (*str >= 'a')
			k = *str - 'a' + 10;
		else if (*str >= 'A')
			k = *str - 'A' + 10;
		if (k < 0 || k >= base)
			break;
		temp = muladd1(z, (digit)base, (digit)k);
		erase(z);
		z = temp;
	}
	if (sign < 0 && z != 0 && num_size(z) != 0)
		num_size(z) = -(num_size(z));
	if (pend)
		*pend = str;
	return huge_normalize(z);
}

/* Long division with remainder, top-level routine */

int number::_huge_divrem(number* a, number* b, number* * pdiv, number* * prem)
{
	digsize size_a = ABS(num_size(a)), size_b = ABS(num_size(b));
	number* z;

	if (!size_b)
		huge_error("divrem(): divide by zero");
	if (size_a < size_b ||
		(size_a == size_b &&
		num_digit(a)[size_a - 1] < num_digit(b)[size_b - 1])) {
			/* |a| < |b|. */
			*pdiv = make(0);
			*prem = dup(a);
			return 0;
	}
	if (size_b == 1) {
		digit rem = 0;
		z = divrem1(a, num_digit(b)[0], &rem);
		if (z == 0)
			return -1;
		*prem = from_long((long)rem);
	} else {
		z = x_divrem (a, b, prem);
		if (z == 0)
			return -1;
	}
	/* Set the signs.
	The quotient z has the sign of a*b;
	the remainder r has the sign of a,
	so a = b*z + r. */
	if ((num_size(a) < 0) != (num_size(b) < 0))
		num_size(z) = -(num_size(z));
	if (num_size(a) < 0 && num_size(*prem) != 0)
		num_size(*prem) = -(num_size(*prem));
	*pdiv = z;
	return 0;
}

/* Unsigned long division with remainder -- the algorithm */

number* number::x_divrem(number* v1, number* w1, number* * prem)
{
	digsize size_v = ABS(num_size(v1)), size_w = ABS(num_size(w1));
	digit d = (digit)((twodigits)BASE / (num_digit(w1)[size_w - 1] + 1));
	number* v = muladd1(v1, d);
	number* w = muladd1(w1, d);
	number* a;
	digsize j, k;

	if (v == 0 || w == 0) {
		erase(v);
		erase(w);
		return 0;
	}
	huge_assert(size_v >= size_w && size_w > 1);	/* Assert checks by div() */
	huge_assert(size_w == ABS(num_size(w)));	/* That's how d was calculated */

	size_v = ABS(num_size(v));
	a = make(size_v - size_w + 1);

	for (j = size_v, k = num_size(a) - 1; a != 0 && k >= 0; --j, --k) {
		digit vj = (j >= size_v) ? 0 : num_digit(v)[j];
		twodigits q;
		stwodigits carry = 0;
		digsize i;

		if (vj == num_digit(w)[size_w - 1])
			q = MASK;
		else
			q = (((twodigits)vj << SHIFT) + num_digit(v)[j - 1]) /
			num_digit(w)[size_w - 1];

		while (num_digit(w)[size_w - 2] * q >
			((
			((twodigits)vj << SHIFT)
			+ num_digit(v)[j - 1]
		- q * num_digit(w)[size_w - 1]
		) << SHIFT)
			+ num_digit(v)[j - 2])
			--q;

		for (i = 0; i < size_w && i + k < size_v; ++i) {
			twodigits z = num_digit(w)[i] * q;
			digit zz = (digit)(z >> SHIFT);
			carry += num_digit(v)[i + k] - z
				+ ((twodigits)zz << SHIFT);
			num_digit(v)[i + k] = (digit)(carry & MASK);
			carry = (carry >> SHIFT) - zz;
		}

		if (i + k < size_v) {
			carry += num_digit(v)[i + k];
			num_digit(v)[i + k] = 0;
		}
		if (carry == 0)
			num_digit(a)[k] = (digit)q;
		else {
			huge_assert(carry == -1);
			num_digit(a)[k] = (digit)q - 1;
			carry = 0;
			for (i = 0; i < size_w && i + k < size_v; ++i) {
				carry += num_digit(v)[i + k] + num_digit(w)[i];
				num_digit(v)[i + k] = (digit)(carry & MASK);
				carry >>= SHIFT;
			}
		}
	}				/* for j, k */

	if (a == 0)
		*prem = 0;
	else {
		a = huge_normalize(a);
		*prem = divrem1(v, d, &d);
		/* d receives the (unused) remainder */
		if (*prem == 0) {
			erase(a);
			a = 0;
		}
	}
	erase(v);
	erase(w);
	return a;
}

int  number::compare(number* a, number* b)
{
	int sign;

	if (num_size(a) != num_size(b)) {
		if (ABS(num_size(a)) == 0 && ABS(num_size(b)) == 0)
			sign = 0;
		else
			sign = (int)(num_size(a) - num_size(b));
	} else {
		digsize i = ABS(num_size(a));
		while (--i >= 0 && num_digit(a)[i] == num_digit(b)[i]);
		if (i < 0)
			sign = 0;
		else {
			sign = (int)(num_digit(a)[i] - num_digit(b)[i]);
			if (num_size(a) < 0)
				sign = -sign;
		}
	}
	return sign < 0 ? -1 : sign > 0 ? 1 : 0;
}

/* Add the absolute values of two long integers. */

number* number::x_add(number* a, number* b)
{
	digsize size_a = ABS(num_size(a)), size_b = ABS(num_size(b));
	number* z;
	digsize i;
	digit carry = 0;

	/* Ensure a is the larger of the two: */
	if (size_a < size_b) {
		{
			number* temp = a;
			a = b;
			b = temp;
		}
		{
			digsize size_temp = size_a;
			size_a = size_b;
			size_b = size_temp;
		}
	}
	z = make(size_a + 1);
	for (i = 0; i < size_b; ++i) {
		carry += num_digit(a)[i] + num_digit(b)[i];
		num_digit(z)[i] = carry & MASK;
		/* The following assumes unsigned shifts don't
		propagate the sign bit. */
		carry >>= SHIFT;
	}
	for (; i < size_a; ++i) {
		carry += num_digit(a)[i];
		num_digit(z)[i] = carry & MASK;
		carry >>= SHIFT;
	}
	num_digit(z)[i] = carry;
	return huge_normalize(z);
}

/* Subtract the absolute values of two integers. */

number* number::x_sub(number* a, number* b)
{
	digsize size_a = ABS(num_size(a)), size_b = ABS(num_size(b));
	number* z;
	digsize i;
	int sign = 1;
	digit borrow = 0;

	/* Ensure a is the larger of the two: */
	if (size_a < size_b) {
		sign = -1;
		{
			number* temp = a;
			a = b;
			b = temp;
		}
		{
			digsize size_temp = size_a;
			size_a = size_b;
			size_b = size_temp;
		}
	} else if (size_a == size_b) {
		/* Find highest digit where a and b differ: */
		i = size_a;
		while (--i >= 0 && num_digit(a)[i] == num_digit(b)[i]);
		if (i < 0)
			return make(0);
		if (num_digit(a)[i] < num_digit(b)[i]) {
			sign = -1;
			{
				number* temp = a;
				a = b;
				b = temp;
			}
		}
		size_a = size_b = i + 1;
	}
	z = make(size_a);
	for (i = 0; i < size_b; ++i) {
		/* The following assumes unsigned arithmetic
		works module 2**N for some N>SHIFT. */
		borrow = num_digit(a)[i] - num_digit(b)[i] - borrow;
		num_digit(z)[i] = borrow & MASK;
		borrow >>= SHIFT;
		borrow &= 1;		/* Keep only one sign bit */
	}
	for (; i < size_a; ++i) {
		borrow = num_digit(a)[i] - borrow;
		num_digit(z)[i] = borrow & MASK;
		borrow >>= SHIFT;
	}
	huge_assert(borrow == 0);
	if (sign < 0)
		num_size(z) = -(num_size(z));
	return huge_normalize(z);
}

number* number::add(number* a, number* b)
{
	number* z;

	if (num_size(a) < 0) {
		if (num_size(b) < 0) {
			z = x_add(a, b);
			if (z != 0 && num_size(z) != 0)
				num_size(z) = -(num_size(z));
		} else
			z = x_sub(b, a);
	} else {
		if (num_size(b) < 0)
			z = x_sub(a, b);
		else
			z = x_add(a, b);
	}
	return (number*)z;
}

number* number::sub(number* a, number* b)
{
	number* z;

	if (num_size(a) < 0) {
		if (num_size(b) < 0)
			z = x_sub(a, b);
		else
			z = x_add(a, b);
		if (z != 0 && num_size(z) != 0)
			num_size(z) = -(num_size(z));
	} else {
		if (num_size(b) < 0)
			z = x_add(a, b);
		else
			z = x_sub(a, b);
	}
	return (number*)z;
}

number* number::mul(number* a, number* b)
{
	digsize size_a;
	digsize size_b;
	number* z;
	digsize i;

	size_a = ABS(num_size(a));
	size_b = ABS(num_size(b));
	z = make(size_a + size_b);
	for (i = 0; i < num_size(z); ++i)
		num_digit(z)[i] = 0;
	for (i = 0; i < size_a; ++i) {
		twodigits carry = 0;
		twodigits f = num_digit(a)[i];
		digsize j;
		for (j = 0; j < size_b; ++j) {
			carry += num_digit(z)[i + j] + num_digit(b)[j] * f;
			num_digit(z)[i + j] = (digit)(carry & MASK);
			carry >>= SHIFT;
		}
		for (; carry != 0; ++j) {
			huge_assert(i + j < num_size(z));
			carry += num_digit(z)[i + j];
			num_digit(z)[i + j] = (digit)(carry & MASK);
			carry >>= SHIFT;
		}
	}
	if (num_size(a) < 0)
		num_size(z) = -(num_size(z));
	if (num_size(b) < 0)
		num_size(z) = -(num_size(z));
	return (number*)huge_normalize(z);
}

/* The / and % operators are now defined in terms of divmod().
The expression a mod b has the value a - b*floor(a/b).
The huge_divrem function gives the remainder after division of
|a| by |b|, with the sign of a.  This is also expressed
as a - b*trunc(a/b), if trunc truncates towards zero.
Some examples:
a     b      a rem b         a mod b
13    10      3               3
-13   10     -3               7
13   -10      3              -7
-13  -10     -3              -3
So, to get from rem to mod, we have to add b if a and b
have different signs.  We then subtract one from the 'divisor'
part of the outcome to keep the invariant intact. */

int number::l_divmod(number* v, number* w, number* * pdiv, number* * pmod)
{
	number* divisor, *mod;

	if (_huge_divrem (v, w, &divisor, &mod) < 0)
		return -1;
	if ((num_size(mod) < 0 && num_size(w) > 0) ||
		(num_size(mod) > 0 && num_size(w) < 0)) {
			number* temp;
			number* one;
			temp =(number*)add(mod, w);
			erase(mod);
			mod = temp;
			if (mod == 0) {
				erase(divisor);
				return -1;
			}
			one = from_long(1L);
			if ((temp =(number*)sub(divisor, one)) == 0) {
				erase(mod);
				erase(divisor);
				erase(one);
				return -1;
			}
			erase(one);
			erase(divisor);
			divisor = temp;
	}
	*pdiv = divisor;
	*pmod = mod;
	return 0;
}

number* number::div(number* v, number* w)
{
	number* divisor, *mod;
	if (l_divmod(v, w, &divisor, &mod) < 0)
		return 0;
	erase(mod);
	return (number*)divisor;
}

number* number::mod(number* v, number* w)
{
	number* divisor, *mod;
	if (l_divmod(v, w, &divisor, &mod) < 0)
		return 0;
	erase(divisor);
	return (number*)mod;
}

number* number::divmod(number* v, number* w, number* * remainder)
{
	number* divisor, *mod;
	if (l_divmod(v, w, &divisor, &mod) < 0)
		return 0;
	if (remainder)
		*remainder = mod;
	return divisor;
}

number* number::powmod(number* a, number* b, number* c)
{
	number* z = 0, *divisor = 0, *mod = 0;
	digsize size_b, i;

	a = dup(a);
	size_b = num_size(b);
	if (size_b < 0) {
		huge_error("pow(): long integer to the negative power");
		return 0;
	}
	z =(number*)from_long(1L);
	for (i = 0; i < size_b; ++i) {
		digit bi = num_digit(b)[i];
		int j;

		for (j = 0; j < SHIFT; ++j) {
			number* temp = 0;

			if (bi & 1) {
				temp =(number*)mul(z, a);
				erase(z);
				if (c != 0 && temp != 0) {
					l_divmod(temp, c, &divisor, &mod);
					erase(divisor);
					erase(temp);
					temp = mod;
				}
				z = temp;
				if (z == 0)
					break;
			}
			bi >>= 1;
			if (bi == 0 && i + 1 == size_b)
				break;
			temp =(number*)mul(a, a);
			erase(a);
			if ((number*)c != 0 && temp != 0) {
				l_divmod(temp, c, &divisor, &mod);
				erase(divisor);
				erase(temp);
				temp = mod;
			}
			a = temp;
			if (a == 0) {
				erase(z);
				z = 0;
				break;
			}
		}
		if (a == 0 || z == 0)
			break;
	}
	erase(a);
	if ((number*)c != 0 && z != 0) {
		l_divmod(z, c, &divisor, &mod);
		erase(divisor);
		erase(z);
		z = mod;
	}
	return (number*)z;
}

number* number::pow(number* a, number* b)
{
	return powmod(a, b, 0);
}

number* number::invert(number* v)
{
	/* Implement ~x as -(x+1) */
	number* x;
	number* w;
	w =(number*)from_long(1L);
	if (w == 0)
		return 0;
	x =(number*)add(v, w);
	erase(w);
	if (x == 0)
		return 0;
	if (num_size(x) != 0)
		num_size(x) = -(num_size(x));
	return (number*)x;
}

number* number::neg(number* v)
{
	number* z;
	digsize i, n;
	n = ABS(num_size(v));
	/* -0 == 0 */
	if (!n)
		return dup(v);
	z = make(n);
	for (i = 0; i < n; i++)
		num_digit(z)[i] = num_digit(v)[i];
	num_size(z) = -(num_size(v));
	return (number*)z;
}

number* number::abs(number* v)
{
	if (num_size(v) < 0)
		return neg(v);
	else
		return dup(v);
}

bool number::isnonzero(number* v)
{
	return v && num_size(v) != 0;
}

number* number::rshift(number* a, digsize shiftby)
{
	number* z;
	digsize newsize, wordshift, loshift, hishift, i, j;
	digit lomask, himask;

	if (num_size(a) < 0) {
		/* Right shifting negative numbers is harder */
		number* a1, *a2, *a3;
		a1 =(number*)invert(a);
		if (a1 == 0)
			return 0;
		a2 =(number*)rshift (a1, shiftby);
		erase(a1);
		if (a2 == 0)
			return 0;
		a3 =(number*)invert(a2);
		erase(a2);
		return (number*)a3;
	}
	if (shiftby < 0) {
		huge_error("rshift(): negative shift count");
		return 0;
	}
	wordshift = shiftby / SHIFT;
	newsize = ABS(num_size(a)) - wordshift;
	if (newsize <= 0) {
		z = make(0);
		return (number*)z;
	}
	loshift = shiftby % SHIFT;
	hishift = SHIFT - loshift;
	lomask = ((digit)1 << hishift) - 1;
	himask = MASK ^ lomask;
	z = make(newsize);
	if (num_size(a) < 0)
		num_size(z) = -(num_size(z));
	for (i = 0, j = wordshift; i < newsize; i++, j++) {
		num_digit(z)[i] = (num_digit(a)[j] >> loshift) & lomask;
		if (i + 1 < newsize)
			num_digit(z)[i] |=
			(num_digit(a)[j + 1] << hishift) & himask;
	}
	return (number*)huge_normalize(z);
}

number* number::lshift(number* a, digsize shiftby)
{
	/* This version due to Tim Peters */
	number* z;
	digsize oldsize, newsize, wordshift, remshift, i, j;
	twodigits accum;

	if (shiftby < 0) {
		huge_error("lshift(): negative shift count");
		return 0;
	}
	/* wordshift, remshift = divmod(shiftby, SHIFT) */
	wordshift = (digsize) shiftby / SHIFT;
	remshift = (digsize) shiftby - wordshift * SHIFT;

	oldsize = ABS(num_size(a));
	newsize = oldsize + wordshift;
	if (remshift)
		++newsize;
	z = make(newsize);
	if (num_size(a) < 0)
		num_size(z) = -(num_size(z));
	for (i = 0; i < wordshift; i++)
		num_digit(z)[i] = 0;
	accum = 0;
	for (i = wordshift, j = 0; j < oldsize; i++, j++) {
		accum |= num_digit(a)[j] << remshift;
		num_digit(z)[i] = (digit)(accum & MASK);
		accum >>= SHIFT;
	}
	if (remshift)
		num_digit(z)[newsize - 1] = (digit)accum;
	else
		huge_assert(!accum);
	return (number*)huge_normalize(z);
}


/* Bitwise and/xor/or operations */

/* op = '&', '|', '^' */
number* number::huge_bitwise(number* a, int op, number* b)
{
	digit maska, maskb;		/* 0 or MASK */
	int negz;
	digsize size_a, size_b, size_z;
	number* z;
	digsize i;
	digit diga, digb;
	number* v;

	if (num_size(a) < 0) {
		a =(number*)invert(a);
		maska = MASK;
	} else {
		a = dup(a);
		maska = 0;
	}
	if (num_size(b) < 0) {
		b =(number*)number::invert(b);
		maskb = MASK;
	} else {
		b = dup(b);
		maskb = 0;
	}

	size_a = num_size(a);
	size_b = num_size(b);
	size_z = MAX(size_a, size_b);
	z = make(size_z);
	if (a == 0 || b == 0) {
		erase(a);
		erase(b);
		erase(z);
		return 0;
	}
	negz = 0;
	switch (op) {
		case '^':
			if (maska != maskb) {
				maska ^= MASK;
				negz = -1;
			}
			break;
		case '&':
			if (maska && maskb) {
				op = '|';
				maska ^= MASK;
				maskb ^= MASK;
				negz = -1;
			}
			break;
		case '|':
			if (maska || maskb) {
				op = '&';
				maska ^= MASK;
				maskb ^= MASK;
				negz = -1;
			}
			break;
	}

	for (i = 0; i < size_z; ++i) {
		diga = (i < size_a ? num_digit(a)[i] : 0) ^ maska;
		digb = (i < size_b ? num_digit(b)[i] : 0) ^ maskb;
		switch (op) {
	case '&':
		num_digit(z)[i] = diga & digb;
		break;
	case '|':
		num_digit(z)[i] = diga | digb;
		break;
	case '^':
		num_digit(z)[i] = diga ^ digb;
		break;
		}
	}

	erase(a);
	erase(b);
	z = huge_normalize(z);
	if (negz == 0)
		return (number*)z;
	v = invert(z);
	erase(z);
	return v;
}

number* number::and(number* a, number* b)
{
	return huge_bitwise (a, '&', b);
}

number* number::xor(number* a, number* b)
{
	return huge_bitwise (a, '^', b);
}

number* number::or(number* a, number* b)
{
	return huge_bitwise (a, '|', b);
}

char* number::oct(number* v)
{
	return format (v, 8);
}

char* number::hex(number* v)
{
	return format (v, 16);
}

char* number::dec(number* v)
{
	return format (v, 10);
}

void number::xhuge_log(number* h, char *msg, char *file, int line)
{
	static FILE *f = 0;
	char *p = 0;
	if (!f)
		huge_assert(!fopen_s (&f, "huge.log", "w"));
	fprintf (f, "%s: %d:\n%s: %s\n", file, line, msg, p = hex(h));
	fflush (f);
	if (p)
		free (p);
}

numptr::numptr(digsize size)
{
	objectRef = number::make(size);
}

numptr::~numptr()
{
	number::erase(objectRef);
}

numptr& numptr::operator=(number* right) {
	if (right != objectRef) {
		if (objectRef) {
			number::erase(objectRef);
		}
		objectRef = right;
	}
	return *this;
}
