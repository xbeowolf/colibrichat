/******************************************************************************
*                                                                             *
* Cyclic redundancy code                                                      *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2010. All rights reserved.       *
*                                                                             *
******************************************************************************/

#pragma once
#ifndef _CRC_
#define _CRC_

#include <tchar.h>

typedef unsigned short CRC16;
typedef unsigned int   CRC32;

#ifdef __cplusplus
extern "C" {
#endif

// featuring Julio Jerez
CRC32 CRCJJ(const char *string, unsigned crc = 0);
CRC32 wCRCJJ(const wchar_t *string, unsigned crc = 0);
#ifdef UNICODE
#define tCRCJJ wCRCJJ
#else
#define tCRCJJ CRCJJ
#endif

// IEEE 802.3 - V.42, MPEG-2, PNG, POSIX cksum
// Polynomial: x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
CRC32 CRC32IEEE(const void* data, size_t len, CRC32 crc = 0xFFFFFFFF);
CRC32 CRC32IEEEtbl(const void* data, size_t len, CRC32 crc = 0xFFFFFFFF);

// Castagnoli - iSCSI, G.hn payload
// Polynomial: x^32 + x^28 + x^27 + x^26 + x^25 + x^23 + x^22 + x^20 + x^19 + x^18 + x^14 + x^13 + x^11 + x^10 + x^9 + x^8 + x^6 + 1
CRC32 CRC32C(const void* data, size_t len, CRC32 crc = 0xFFFFFFFF);

// Koopman
// Polynomial: x^32 + x^30 + x^29 + x^28 + x^26 + x^20 + x^19 + x^17 + x^16 + x^15 + x^11 + x^10 + x^7 + x^6 + x^4 + x^2 + x + 1
CRC32 CRC32K(const void* data, size_t len, CRC32 crc = 0xFFFFFFFF);

// aviation; AIXM
// Polynomial: Polynomial: x^32 + x^31 + x^24 + x^22 + x^16 + x^14 + x^8 + x^7 + x^5 + x^3 + x + 1
CRC32 CRC32Q(const void* data, size_t len, CRC32 crc = 0xFFFFFFFF);

// CRC-CCITT - X.25, HDLC, XMODEM, Bluetooth, SD, many others
// Polynomial: x^16 + x^12 + x^5 + 1
CRC16 CRC16CCITTtbl(const void* data, size_t len, CRC16 crc = 0xFFFF);

#ifdef __cplusplus
};
#endif

#endif // _CRC_