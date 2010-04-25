/******************************************************************************
*                                                                             *
* kcp.h -- Colibri Chat Protocol constants and declarations                   *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2009. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _COLIBRIPROTOCOL_
#define _COLIBRIPROTOCOL_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

// Project
#include "bnp.h"

#pragma endregion

//-----------------------------------------------------------------------------

#define CCP_PORT                       7531

// Current engine version
#define BNP_ENGINEVERSSTRW             MAKEVERSIONSTR(1, 2, 4, 1)
#define BNP_ENGINEVERSSTRA             MAKEVERSIONSTR(1, 2, 4, 1)
#define BNP_ENGINEVERSNUM              MAKEVERSIONNUM(1, 2, 4, 1)
#define BNP_ENGINEVERSMIN              MAKEVERSIONNUM(1, 2, 3, 2)
#ifdef UNICODE
#define BNP_ENGINEVERSSTR              BNP_ENGINEVERSSTRW
#else
#define BNP_ENGINEVERSSTR              BNP_ENGINEVERSSTRA
#endif

//-----------------------------------------------------------------------------

// Reserved contacts identifiers
#define CRC_SERVER                     0xa67413a6
#define NAME_SERVER                    TEXT("Server")
#define CRC_LIST                       0xfc089517
#define NAME_LIST                      TEXT("Channels")
#define CRC_NONAME                     0x265cf8a4
#define NAME_NONAME                    TEXT("Noname")
#define CRC_ANONYMOUS                  0xbec1fbcd
#define NAME_ANONYMOUS                 TEXT("Anonymous")
#define CRC_GOD                        0x0dfc6129
#define NAME_GOD                       TEXT("God")
#define CRC_DEVIL                      0xf62ea53e
#define NAME_DEVIL                     TEXT("Devil")

//-----------------------------------------------------------------------------

//
// Chat base
//

// Sets metrics from server
#define CCPM_METRICS                   100

// Creates new nick
#define CCPM_NICK                      101
#define NICK_OK                        0
#define NICK_TAKEN                     1
#define NICK_TAKENCHANNEL              2
#define NICK_TAKENUSER                 3

// Changing password
#define CCPM_PASSWORD                  102 // reserved

// Channels list
#define CCPM_LIST                      103

// User information, i.e. name, IP, creation time
#define CCPM_USERINFO                  104

// Join to channel or to private talk
#define CCPM_JOIN                      105
#define CHAN_OK                        0
#define CHAN_ALREADY                   1
#define CHAN_BADPASS                   2
#define CHAN_DENY                      3
#define CHAN_LIMIT                     4
#define CHAN_ABSENT                    5

// Part channel or private talk
#define CCPM_PART                      106
#define PART_KICK                      0
#define PART_LEAVE                     1
#define PART_DISCONNECT                2

// Indicate application activation
#define CCPM_ONLINE                    107

// Status components
#define CCPM_STATUS                    108
#define STATUS_MODE                    0x0001
#define STATUS_IMG                     0x0002
#define STATUS_MSG                     0x0004
#define STATUS_GOD                     0x0008
#define STATUS_DEVIL                   0x0010

// Say to channel or to private talk
#define CCPM_SAY                       109

// Change channel topic
#define CCPM_TOPIC                     110

// Changing channel options: limits, colors, etc.
#define CCPM_CHANOPTIONS               111
#define CHANOP_AUTOSTATUS              1
#define CHANOP_LIMIT                   2
#define CHANOP_HIDDEN                  3
#define CHANOP_ANONYMOUS               4
#define CHANOP_BACKGROUND              5

// Change user status on channel
#define CCPM_ACCESS                    112

// Message or alert
#define CCPM_MESSAGE                   113
#define MESSAGE_IGNORE                 0
#define MESSAGE_SENT                   1
#define MESSAGE_SAVED                  2

// Beep
#define CCPM_BEEP                      114

// Send windows clipboard content
#define CCPM_CLIPBOARD                 115

// Creates splash-window with given text content
#define CCPM_SPLASHRTF                 116

//-----------------------------------------------------------------------------

#endif // _COLIBRIPROTOCOL_