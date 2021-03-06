// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define  _WIN32_WINNT   0x0501 // _WIN32_WINNT_WINXP
#define NTDDI_VERSION   0x05010300 // NTDDI_WINXPSP3

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// C/C++ RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// STL
#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <sstream>

// Common
#include "sysexcept.h"
#include "JObjects.h"
#include "stringutil.h"
#include "FastDelegate.h"
#include "FastDelegateBind.h"
#include "FastDelegateList.h"

// Windows
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>

//-----------------------------------------------------------------------------

#define __WTEXT(quote) L##quote
#define WTEXT(quote) __WTEXT(quote)
#define __ATEXT(quote) quote
#define ATEXT(quote) __ATEXT(quote)

// The End.
