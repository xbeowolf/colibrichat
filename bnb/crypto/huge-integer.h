#ifndef _HUGE_INT_
#define _HUGE_INT_
#pragma once

// STL
#include <string>

#ifndef NOMINMAX
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif  /* NOMINMAX */

namespace huge {

	//
	// Interface
	//

#ifndef HUGE_INTDEF
#define HUGE_INTDEF
#if defined(_WIN64)
	typedef unsigned __int32 digit;
	typedef unsigned __int64 wdigit;
	typedef unsigned __int64 twodigits;
	typedef union {
		digit arr[2];
		twodigits two;
		struct {
			digit lo, hi;
		} part;
	} compound;
#elif defined(_WIN32)
	typedef unsigned __int16 digit;
	typedef unsigned __int32 wdigit;
	typedef unsigned __int32 twodigits;
	typedef union {
		digit arr[2];
		twodigits two;
		struct {
			digit lo, hi;
		} part;
	} compound;
#else
	typedef unsigned __int8  digit;
	typedef unsigned __int16 wdigit;
	typedef unsigned __int16 twodigits;
	typedef union {
		digit arr[2];
		twodigits two;
		struct {
			digit lo, hi;
		} part;
	} compound;
#endif
#endif

	template<unsigned intbits> class integer {
	public:

		typedef integer* pinteger;
		static const unsigned digitbits = sizeof(digit)*8;
		static const unsigned digitsnum = (intbits + sizeof(digit)*8 - 1) / sizeof(digit) / 8;
		static const unsigned maxdecimaldigits = (unsigned)(intbits/3.3219280948873623478703194294894f + 1);

		bool iszero(unsigned begin = 0, unsigned end = digitsnum) const;
		bool isnonzero(unsigned begin = 0, unsigned end = digitsnum) const;
		void zero(unsigned begin = 0, unsigned end = digitsnum);
		unsigned useddigits() const;
		unsigned usedbytes() const;
		unsigned usedbits() const;
		int  compare(const integer& right) const;

		template<typename T>
		void stoi(const T* str, int base = 16, const T* charset = 0, bool solid = false);
		template<typename T>
		std::basic_string<T> tostr(digit base = 16, const T* charset = 0) const;
		template<typename T> // formats with mask same as "#####-#####-#####"
		std::basic_string<T> format(const T* mask, digit base = 16, const T* charset = 0) const;

		integer& muladd(digit mul, digit extra = 0);
		integer& divrem(digit div, digit& rem);

		bool operator ! () const;
		bool operator == (const integer& right) const;
		bool operator != (const integer& right) const;
		bool operator > (const integer& right) const;
		bool operator < (const integer& right) const;
		bool operator >= (const integer& right) const;
		bool operator <= (const integer& right) const;

		template<unsigned t_sizeR>
		integer& operator = (const integer<t_sizeR>& right);
		integer& operator = (digit right);
		template<typename T>
		integer& operator = (const T* str);

		integer& operator |= (const integer& right);
		integer& operator &= (const integer& right);
		integer& operator ^= (const integer& right);

		integer& operator += (const integer& right);
		integer& operator += (digit right);
		integer& operator -= (const integer& right);
		integer& operator -= (digit right);
		integer& operator *= (const integer& right);
		integer& operator *= (digit right);
		integer& operator /= (const integer& right);
		integer& operator /= (digit right);
		integer& operator %= (const integer& right);
		integer& operator %= (digit right);

		integer operator ~ () const;
		integer operator | (const integer& right) const;
		integer operator & (const integer& right) const;
		integer operator ^ (const integer& right) const;

		integer operator + (const integer& right) const;
		integer operator + (digit right) const;
		integer operator - (const integer& right) const;
		integer operator - (digit right) const;
		integer operator * (const integer& right) const;
		integer operator * (digit right) const;
		integer operator / (const integer& right) const;
		integer operator / (digit right) const;
		integer operator % (const integer& right) const;
		digit   operator % (digit right) const;

	public:

		digit m_data[digitsnum];
	};

	//
	// Implementation
	//

	template<unsigned intbits>
	bool integer<intbits>::iszero(unsigned begin, unsigned end) const {
		unsigned i;
		for (i = begin; i < end && !m_data[i]; i++) {}
		return i >= end;
	}

	template<unsigned intbits>
	bool integer<intbits>::isnonzero(unsigned begin, unsigned end) const {
		unsigned i;
		for (i = begin; i < end && !m_data[i]; i++) {}
		return i < end;
	}

	template<unsigned intbits>
	void integer<intbits>::zero(unsigned begin, unsigned end) {
		memset(m_data + begin, 0, (end - begin)*sizeof(digit));
	}

	template<unsigned intbits>
	unsigned integer<intbits>::useddigits() const {
		unsigned i;
		for (i = digitsnum-1; i != unsigned(-1) && !m_data[i]; i--) {}
		return i+1;
	}

	template<unsigned intbits>
	unsigned integer<intbits>::usedbytes() const {
		unsigned i;
		unsigned char* ptr = (unsigned char*)m_data;
		for (i = sizeof(m_data)-1; i != unsigned(-1) && !ptr[i]; i--) {}
		return i+1;
	}

	template<unsigned intbits>
	unsigned integer<intbits>::usedbits() const {
		unsigned i;
		for (i = digitsnum-1; i != unsigned(-1) && !m_data[i]; i--) {}
		if (i != unsigned(-1)) {
			digit c = m_data[i];
			i *= digitbits;
			while (c) {
				c >>= 1; i++;
			}
			return i;
		} else return 0;
	}

	template<unsigned intbits>
	int  integer<intbits>::compare(const integer& right) const {
		unsigned i;
		for (i = digitsnum - 1; i && m_data[i] == right.m_data[i]; i--) {}
		return (int)(m_data[i] - right.m_data[i]);
	}

	template<unsigned intbits> template<typename T>
	void integer<intbits>::stoi(const T* str, int base, const T* charset, bool solid) {
		static const T defcharset[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0};
		// read header
		if (str[0] && str[1]) {
			if (str[0] == '0') {
				if (str[1] == 'x' || str[1] == 'X') {
					base = 16;
					str += 2;
				} else {
					base = 8;
					str += 1;
				}
			} else if (str[1] == '#') {
				base = str[0] - '0';
				str += 2;
			} else if (str[2] && str[2] == '#') {
				base = (str[0] - '0')*10 + (str[1] - '0');
				str += 3;
			}
		}
		if (!base) {
			if (charset) base = strlen(charset);
			else base = 10;
		}
		if (!charset) charset = defcharset;
		// read body
		zero();
		digit c;
		const T* pos;
		while (*str) {
			c = *str;
			if (c < '0' || c > '9') c &= 0xDF;
			for (pos = charset; *pos && *pos != c; pos++) {}
			if (*pos) {
				c = (digit)(pos - charset);
				*this *= base;
				*this += c;
			} else if (solid) break;
			str++;
		}
	}

	template<unsigned intbits> template<typename T>
	std::basic_string<T> integer<intbits>::tostr(digit base, const T* charset) const {
		static const T defcharset[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0};
		if (!base && charset) for (base = 0; charset[base]; base++) {}
		_ASSERT(base >= 2 && base <= 36);
		if (!charset) charset = defcharset;
		integer val(*this);
		digit rem;
		// calculate a rough upper bound for the length of the string
		unsigned n;
#ifndef _INC_MATH
		unsigned bits;
		for (n = base, bits = 0; n > 1; n >>= 1, bits++) {}
		n = (sizeof(m_data)*8)/bits + 1;
#else
		n = (unsigned)(intbits*0.69314718055994530941723212145818f/log((float)base) + 1);
#endif
#ifdef ORIGINAL_BEHAVIOUR
		n += 3;
#endif
		// allocate temporary string
		std::basic_string<T> str(n, 0);
		T* ptr = (T*)str.data() + n;
		// write body
		do {
			val.divrem(base, rem);
			*--ptr = charset[rem];
		} while (val.isnonzero());
		// write header
#ifdef ORIGINAL_BEHAVIOUR
		if (base == 8) {
			if (isnonzero())
				*--ptr = '0';
		} else if (base == 16) {
			*--ptr = 'x';
			*--ptr = '0';
		} else if (base != 10) {
			*--ptr = '#';
			*--ptr = '0' + base % 10;
			if (base > 10)
				*--ptr = '0' + base / 10;
		}
#endif
		return ptr;
	}

	template<unsigned intbits> template<typename T>
	std::basic_string<T> integer<intbits>::format(const T* mask, digit base, const T* charset) const
	{
		static const T defcharset[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0};
		if (!base && charset) for (base = 0; charset[base]; base++) {}
		_ASSERT(base >= 2 && base <= 36);
		if (!charset) charset = defcharset;
		integer val(*this);
		digit rem;

		std::basic_string<T> res = mask;
		for (int i = (int)res.size()-1; i >= 0; i--) {
			if (res[i] == '#') {
				if (val.isnonzero()) {
					val.divrem(base, rem);
					res[i] = charset[rem];
				} else {
					res[i] = charset[0];
				}
			}
		}
		return res;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::muladd(digit mul, digit extra)
	{
		twodigits carry = extra;
		for (unsigned i = 0; i < digitsnum; i++) {
			carry += (twodigits)m_data[i] * mul;
			m_data[i] = (digit)(carry);
			carry >>= digitbits;
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::divrem(digit div, digit& rem) {
		_ASSERT(div != 0);
		twodigits carry = 0;
		for (unsigned i = digitsnum; --i != unsigned(-1);) {
			carry = (carry << digitbits) + m_data[i];
			m_data[i] = (digit)(carry / div);
			carry %= div;
		}
		rem = (digit)carry;
		return *this;
	}

	template<unsigned intbits>
	bool integer<intbits>::operator ! () const {
		return !iszero();
	}

	template<unsigned intbits>
	bool integer<intbits>::operator == (const integer& right) const {
		return memcmp(m_data, right.m_data, sizeof(m_data)) == 0;
	}

	template<unsigned intbits>
	bool integer<intbits>::operator != (const integer& right) const {
		return memcmp(m_data, right.m_data, sizeof(m_data)) != 0;
	}

	template<unsigned intbits>
	bool integer<intbits>::operator > (const integer& right) const {
		return compare(right) > 0;
	}

	template<unsigned intbits>
	bool integer<intbits>::operator < (const integer& right) const {
		return compare(right) < 0;
	}

	template<unsigned intbits>
	bool integer<intbits>::operator >= (const integer& right) const {
		return compare(right) >= 0;
	}

	template<unsigned intbits>
	bool integer<intbits>::operator <= (const integer& right) const {
		return compare(right) <= 0;
	}

	template<unsigned intbits> template<unsigned t_bitsR>
	integer<intbits>& integer<intbits>::operator = (const integer<t_bitsR>& right) {
		unsigned size = min(digitsnum, integer<t_bitsR>::digitsnum);
		memmove_s(m_data, size*sizeof(digit), right.m_data, size*sizeof(digit));
		zero(size);
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator = (digit right) {
		m_data[0] = right;
		zero(1);
		return *this;
	}

	template<unsigned intbits> template<typename T>
	integer<intbits>& integer<intbits>::operator = (const T* str) {
		stoi(str);
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator |= (const integer& right) {
		for (unsigned i = 0; i < digitsnum; i++) {
			m_data[i] |= right.m_data[i];
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator &= (const integer& right) {
		for (unsigned i = 0; i < digitsnum; i++) {
			val.m_data[i] &= right.m_data[i];
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator ^= (const integer& right) {
		for (unsigned i = 0; i < digitsnum; i++) {
			val.m_data[i] ^= right.m_data[i];
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator += (const integer& right) {
		compound op;
		op.two = 0;
		for (unsigned i = 0; i < digitsnum; i++) {
			op.two += m_data[i] + right.m_data[i];
			m_data[i] = op.part.lo;
			op.two >>= digitbits;
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator += (digit right) {
		compound op;
		op.two = right;
		for (unsigned i = 0; i < digitsnum && op.part.lo; i++) {
			op.two += m_data[i];
			m_data[i] = op.part.lo;
			op.two >>= digitbits;
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator -= (const integer& right) {
		compound op;
		op.two = 0;
		for (unsigned i = 0; i < digitsnum; i++) {
			op.two += m_data[i] - right.m_data[i];
			m_data[i] = op.part.lo;
			op.two >>= digitbits;
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator -= (digit right) {
		compound op;
		op.two = -right;
		for (unsigned i = 0; i < digitsnum && op.part.lo; i++) {
			op.two += m_data[i];
			m_data[i] = op.part.lo;
			op.two >>= digitbits;
		}
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator *= (const integer& right) {
		return *this = *this * right;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator *= (digit right) {
		muladd(right);
		return *this;
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator /= (digit right) {
		digit rem;
		return divrem(right, rem);
	}

	template<unsigned intbits>
	integer<intbits>& integer<intbits>::operator %= (digit right) {
		digit rem;
		divrem(right, rem);
		return *this = rem;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator ~ () const {
		integer val;
		for (unsigned i = 0; i < digitsnum; i++) {
			val.m_data[i] = ~m_data[i];
		}
		return val;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator | (const integer& right) const {
		integer val(*this);
		return val |= right;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator & (const integer& right) const {
		integer val(*this);
		return val &= right;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator ^ (const integer& right) const {
		integer val(*this);
		return val ^= right;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator + (const integer& right) const {
		integer val(*this);
		return val += right;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator + (digit right) const {
		integer val(*this);
		return val += right;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator - (const integer& right) const {
		integer val(*this);
		return val -= right;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator - (digit right) const {
		integer val(*this);
		return val -= right;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator * (const integer& right) const {
		integer val;
		val.zero();
		twodigits carry;
		for (unsigned i = 0; i < digitsnum; i++) {
			carry = 0;
			for (unsigned j = 0; j < digitsnum - i; j++) {
				carry += val.m_data[i + j] + right.m_data[j] * m_data[i];
				val.m_data[i + j] = (digit)carry;
				carry >>= digitbits;
			}
		}
		return val;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator * (digit right) const {
		integer val(*this);
		val.muladd(right);
		return val;
	}

	template<unsigned intbits>
	integer<intbits> integer<intbits>::operator / (digit right) const {
		integer val(*this);
		digit rem;
		val.divrem(right, rem);
		return val;
	}

	template<unsigned intbits>
	digit integer<intbits>::operator % (digit right) const {
		integer val(*this);
		digit rem;
		val.divrem(right, rem);
		return rem;
	}
}; // huge

#endif // _HUGE_INT_
