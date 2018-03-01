/*
** huge-number.h
**
** Arbitrary precision integer library from Python sources.
**
** This is a minor modification of the file "huge-number.h" taken from
** mirrordir-0.10.49 which in turn contains these copyrights ...
**
** $Id: huge.h,v 1.1.1.1 2001/04/12 18:08:01 ndwinton Exp $
**
** Refactoring by Podobashev Dmitry / BEOWOLF, 2009
*/

/* huge-number.h: arbitrary precision integer library from Python sources
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

#ifndef _HUGE_NUM_
#define _HUGE_NUM_

namespace huge {

#ifndef HUGE_INTDEF
#define HUGE_INTDEF
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	// this gives a roughly a 7/4 speed increase with powmod()
	typedef unsigned int  digit;
	typedef unsigned int  wdigit; // digit widened to parameter size
	typedef unsigned long long twodigits;
	typedef   signed long long stwodigits; // signed variant of twodigits
#else
	typedef unsigned short digit;
	typedef unsigned int  wdigit;
	typedef unsigned long twodigits;
	typedef   signed long stwodigits;
#endif
	typedef union {
		digit arr[2];
		twodigits two;
		struct {
			digit lo, hi;
		} part;
	} compound;
#endif
	typedef signed long       digsize;

	const int SHIFT = sizeof(digit)*8 - 1;
	const digit BASE = ((digit)1 << SHIFT);
	const digit MASK = ((int)(BASE - 1));

	class number {
	public:

		/* management */
		static number* make(digsize size);
		static void copy(number* a, number* b);
		static number* dup(number* a);
		static void erase(number*& a);

		/* type conversion */
		static number* from_string(char *str, char **pend, int base);
		static number* from_long(long ival);
		static number* from_unsigned_long(unsigned long ival);
		static long as_long(number* v);
		static unsigned long as_unsigned_long(number* v);

		/* bit manipulation */
		static number* set_bit(number* v, digsize i);
		static void clear_bit(number* v, digsize i);

		/* octet stream */
		static number* from_binary(unsigned char *s, digsize l);
		static char* as_binary(number* a, digsize *l);

		/* formatting */
		static char* format(number* a, int base);
		static char* oct(number* v);
		static char* hex(number* v);
		static char* dec(number* v);

		/* comparison */
		static int  compare(number* a, number* b);
		static bool isnonzero(number* v);

		/* arithmetic */
		static number* add(number* a, number* b);
		static number* sub(number* a, number* b);
		static number* mul(number* a, number* b);
		static number* div(number* v, number* w);
		static number* mod(number* v, number* w);
		static number* divmod(number* v, number* w, number* * remainder /* may be null */ );
		static number* invert(number* v);

		/* exponentiation */
		static number* pow(number* a, number* b);
		static number* powmod(number* a, number* b, number* c);

		/* unary */
		static number* neg(number* v);
		static number* abs(number* v);

		/* shifting */
		static number* rshift(number* a, digsize shiftby);
		static number* lshift(number* a, digsize shiftby);

		/* logical */
		static number* and(number* a, number* b);
		static number* xor(number* a, number* b);
		static number* or(number* a, number* b);

		/* log */
		/* #define huge_log(x,y) xhuge_log(x,y,__FILE__,__LINE__) */
#define huge_log(x,y) 
		static void xhuge_log(number* h, char *msg, char *file, int line);

	protected:

		static number* huge_normalize(number* );
		static number* muladd1(number* , wdigit, wdigit = 0);
		static number* divrem1(number* , wdigit, digit *);
		static number* x_divrem(number* v1, number* w1, number* * prem);

	private:

		// no any constructor available
		number() {}
		number(const number&) {}

		static unsigned char _huge_get_char(number* a, digsize j);
		static int _huge_divrem(number* a, number* b, number* * pdiv, number* * prem);
		static number* x_add(number* a, number* b);
		static number* x_sub(number* a, number* b);
		static int l_divmod(number* v, number* w, number* * pdiv, number* * pmod);
		static number* huge_bitwise(number* a, int op, number* b);

	public:

		digsize m_size;
		digit   m_digit[1]; // size defined by m_size member
	};

	class numptr {
	public:
		explicit numptr(number* x = 0) : objectRef(x) {}
		explicit numptr(digsize size);
		~numptr();

		numptr& operator=(number* right);
		number* __fastcall operator->() const { return objectRef; }
		__fastcall operator number*() const { return objectRef; }
		number& __fastcall operator*() const { return *objectRef; }
		__fastcall operator bool() const { return (objectRef != NULL); }

	protected:
		number* objectRef;
	};
}; // huge

#endif /* ! _HUGE_NUM_ */
