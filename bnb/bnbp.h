/******************************************************************************
*                                                                             *
* bnbp.h -- Beowolf Network Binary Protocol declarations                      *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2006-2010. All rights reserved.  *
*                                                                             *
******************************************************************************/

#ifndef _BNBP_
#define _BNBP_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

#pragma endregion

//-----------------------------------------------------------------------------

// Protocol identification signature
#define BNP_SIGNATURE          "Beowolf network"

//-----------------------------------------------------------------------------

//
// Beowolf Network Protocol transactions header type definition
//

// Primary header
//   m_sizedim member means:
//     0 - no data
//     1 - 1 byte sized data length
//     2 - 2 bytes sized data length
//     3 - 4 bytes sized data length
//   m_hasTrnid - "trnid" existence, if false - transaction is notify/command
//   m_isQuest - transaction is quest or reply
//   m_isCompressed - "sizeuncompr" existance and is data compressed
//   m_isEncrypted - data is encoded
//   m_message - identifies transaction message
typedef struct {
	unsigned short m_sizedim : 2;
	unsigned short m_hasTrnid : 1;
	unsigned short m_isQuest : 1;
	unsigned short m_isCompressed : 1;
	unsigned short m_isReserved : 1;
	unsigned short m_message : 10;
} BNP_HDR2;

//-----------------------------------------------------------------------------
//
// Beowolf Network Protocol messages ids of transactions
//
//-----------------------------------------------------------------------------

//
// Action type
//

// Definitions for compound messages
#define BNPM_COMMAND           0x0000U
#define BNPM_QUEST             0x4000U
#define BNPM_REPLY             0x8000U
#define BNPM_NOTIFY            0xC000U
#define BNPM_ACTION            0xC000U
#define BNPM_MESSAGE           0x3FFFU

// Make compound message
#define COMMAND(msg)           ((msg)|BNPM_COMMAND)
#define QUEST(msg)             ((msg)|BNPM_QUEST)
#define REPLY(msg)             ((msg)|BNPM_REPLY)
#define NOTIFY(msg)            ((msg)|BNPM_NOTIFY)
// Strip compound message components
#define NATIVEACTION(msg)      ((msg)&BNPM_ACTION)
#define NATIVEMESSAGE(msg)     ((msg)&BNPM_MESSAGE)
// Make compound message from primary header
#define BUILTMESSAGE(hdr2)     ((hdr2).m_message|((hdr2).m_hasTrnid?((hdr2).m_isQuest?BNPM_QUEST:BNPM_REPLY):((hdr2).m_isQuest?BNPM_NOTIFY:BNPM_COMMAND)))

#define BNP_COMPRESS_STEP      4096

// Message identifiers can be in range 0 - 1023

//
// Basic messages
//

// Without data
#define BNPM_NULL              0

// Without data
#define BNPM_PING              2

// Checks access to server
// Data structure:
// [quest]
//   char[16] - signature "Remote Hitman"
//   DWORD - version
//   tstring - password to connection
// [reply]
//   LRESULT - 0 - success, 1 or more - deny
//   size_t - access set size
//   WORD[] - access set
#define BNPM_IDENTIFY          5

// Make server as proxy
// Data structure:
// [command]
//   sockaddr_in - socket address structure
//   tstring - password
// [notify]
//   sockaddr_in - new server connected target
#define BNPM_CONNECT           6

// Disconnect remote target from sender
// [command]
// Without data
#define BNPM_DISCONNECT        8

// Application private setting
// Data structure:
// [command, quest, reply, notify]
//   char[16] - application signature
//   UINT - setting identifier
//   private data
#define BNPM_SETTING           10

//-----------------------------------------------------------------------------

#endif // _BNBP_