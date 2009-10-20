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

#define CCP_PORT               7531

// Current engine version
#define BNP_ENGINEVERSSTRW MAKEVERSIONSTR(1, 0, 0, 1)
#define BNP_ENGINEVERSSTRA MAKEVERSIONSTR(1, 0, 0, 1)
#define BNP_ENGINEVERSNUM  MAKEVERSIONNUM(1, 0, 0, 1)
#define BNP_ENGINEVERSMIN  MAKEVERSIONNUM(1, 0, 0, 1)
#ifdef UNICODE
#define BNP_ENGINEVERSSTR  BNP_ENGINEVERSSTRW
#else
#define BNP_ENGINEVERSSTR  BNP_ENGINEVERSSTRA
#endif

//-----------------------------------------------------------------------------

// Reserved contacts identifiers
#define CRC_SERVER             0xa67413a6
#define NAME_SERVER            TEXT("Server")
#define CRC_LIST               0xfc089517
#define NAME_LIST              TEXT("Channels")
#define CRC_NONAME             0x265cf8a4
#define NAME_NONAME            TEXT("Noname")
#define CRC_ANONYMOUS          0xbec1fbcd
#define NAME_ANONYMOUS         TEXT("Anonymous")
#define CI_RESERVEDPOOL        10

//-----------------------------------------------------------------------------

//
// Chat base
//

// Creates new nick
#define CCPM_NICK                      100
#define NICK_OK                        0
#define NICK_TAKEN                     1
#define NICK_TAKENCHANNEL              2
#define NICK_TAKENUSER                 3

// Changing password
#define CCPM_PASSWORD                  101

// Channels list
#define CCPM_LIST                      102

// User information, i.e. name, IP, creation time
#define CCPM_USERINFO                  103

// Join to channel or to private talk
#define CCPM_JOIN                      104
#define CHAN_OK                        0
#define CHAN_ALREADY                   1
#define CHAN_DENY                      2
#define CHAN_LIMIT                     3
#define CHAN_ABSENT                    4

// Part channel or private talk
#define CCPM_PART                      105
#define PART_KICK                      0
#define PART_LEAVE                     1
#define PART_DISCONNECT                2

// Indicate application activation
#define CCPM_ONLINE                    106

// Status components
#define CCPM_STATUS                    107
#define STATUS_MODE                    0x0001
#define STATUS_IMG                     0x0002
#define STATUS_MSG                     0x0004

// Say to channel or to private talk
#define CCPM_SAY                       108

// Change channel topic
#define CCPM_TOPIC                     109

// Changing background colors
#define CCPM_BACKGROUND                110

// Change user status on channel
#define CCPM_ACCESS                    111

// Message or alert
#define CCPM_MESSAGE                   112
#define MESSAGE_IGNORE                 0
#define MESSAGE_SENT                   1
#define MESSAGE_SAVED                  2

// Beep
#define CCPM_BEEP                      113

// Send windows clipboard content
#define CCPM_CLIPBOARD                 114

// Creates splash-window with given text content
#define CCPM_SPLASHRTF                 115

//-----------------------------------------------------------------------------

#endif // _COLIBRIPROTOCOL_