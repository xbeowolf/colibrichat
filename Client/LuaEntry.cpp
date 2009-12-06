
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "dCRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "client.h"

#pragma endregion

using namespace colibrichat;

//-----------------------------------------------------------------------------

#define DECLARE_METHODLUA(typename, methodname) {#methodname, &typename::lua_##methodname}
#define REGISTER_STRING(val, name) lua_pushstring(luaVM, tstrToANSI(val).c_str()), lua_setglobal(luaVM, name)

//-----------------------------------------------------------------------------

static int getInt(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 3);
	if (lua_isstring(luaVM, 1) && lua_isstring(luaVM, 2) && lua_isnumber(luaVM, 3)) {
		std::tstring szSection = ANSIToTstr(lua_tostring(luaVM, 1));
		std::tstring szEntry = ANSIToTstr(lua_tostring(luaVM, 2));
		UINT nDefault = (UINT)lua_tointeger(luaVM, 3);
		UINT result = Profile::GetInt(szSection.c_str(), szEntry.c_str(), nDefault);
		lua_pushinteger(luaVM, result);
	} else {
		lua_pushstring(luaVM, "incorrect argument in function \"getInt\"");
		lua_error(luaVM);
	}
	return 1;
}

static int setInt(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 3);
	if (lua_isstring(luaVM, 1) && lua_isstring(luaVM, 2) && lua_isnumber(luaVM, 3)) {
		std::tstring szSection = ANSIToTstr(lua_tostring(luaVM, 1));
		std::tstring szEntry = ANSIToTstr(lua_tostring(luaVM, 2));
		UINT nValue = (UINT)lua_tointeger(luaVM, 3);
		Profile::WriteInt(szSection.c_str(), szEntry.c_str(), nValue);
	} else {
		lua_pushstring(luaVM, "incorrect argument in function \"setInt\"");
		lua_error(luaVM);
	}
	return 0;
}

static int hasstr(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 2);
	if (lua_isstring(luaVM, 1) && lua_isstring(luaVM, 2)) {
		const char* host = lua_tostring(luaVM, 1);
		const char* entry = lua_tostring(luaVM, 2);
		int i = 0;
		while ((host = strstr(host, entry)) != 0) {
			host += strlen(entry);
			i++;
		}
		lua_pushinteger(luaVM, i);
	} else {
		lua_pushstring(luaVM, "incorrect argument in function \"hasstr\"");
		lua_error(luaVM);
	}
	return 1;
}

int JClient::lua_regFuncs(lua_State *luaVM)
{
	static luaL_Reg methods1[] = {
		{"getInt", &getInt},
		{"setInt", &setInt},
		{0, 0}
	};
	static luaL_Reg methods2[] = {
		{"hasstr", &hasstr},
		{0, 0}
	};
	luaL_register(luaVM, "profile", methods1);
	luaL_register(luaVM, "str", methods2);
	return 0;
}

//-----------------------------------------------------------------------------

const char JClient::className[] = "Client";

CLuaGluer<JClient>::_tRegType JClient::methods[] =
{
	DECLARE_METHODLUA(JClient, regFuncs),
	DECLARE_METHODLUA(JClient, getGlobal),
	DECLARE_METHODLUA(JClient, getVars),
	DECLARE_METHODLUA(JClient, setVars),
	DECLARE_METHODLUA(JClient, PlaySound),
	DECLARE_METHODLUA(JClient, ShowTopic),
	DECLARE_METHODLUA(JClient, saveAutoopen),
	DECLARE_METHODLUA(JClient, openAutoopen),
	DECLARE_METHODLUA(JClient, Log),
	DECLARE_METHODLUA(JClient, HideBaloon),
	DECLARE_METHODLUA(JClient, Connect),
	DECLARE_METHODLUA(JClient, Disconnect),
	DECLARE_METHODLUA(JClient, getConnectCount),
	DECLARE_METHODLUA(JClient, setConnectCount),
	DECLARE_METHODLUA(JClient, getSocket),
	DECLARE_METHODLUA(JClient, checkConnectionButton),
	DECLARE_METHODLUA(JClient, WaitConnectStart),
	DECLARE_METHODLUA(JClient, WaitConnectStop),
	DECLARE_METHODLUA(JClient, MinimizeWindow),
	DECLARE_METHODLUA(JClient, MaximizeWindow),
	DECLARE_METHODLUA(JClient, RestoreWindow),
	DECLARE_METHODLUA(JClient, FlashWindow),
	DECLARE_METHODLUA(JClient, DestroyWindow),
	DECLARE_METHODLUA(JClient, PageEnable),
	DECLARE_METHODLUA(JClient, PageDisable),
	DECLARE_METHODLUA(JClient, PageAppendScript),
	DECLARE_METHODLUA(JClient, Say),
	DECLARE_METHODLUA(JClient, Message),
	DECLARE_METHODLUA(JClient, Alert),
	DECLARE_METHODLUA(JClient, Beep),
	{0, 0}
};

int JClient::lua_getGlobal(lua_State *luaVM) {
	// Register application waves
	REGISTER_STRING(JClientApp::jpApp->strWavMeline, "wavMeline");
	REGISTER_STRING(JClientApp::jpApp->strWavChatline, "wavChatline");
	REGISTER_STRING(JClientApp::jpApp->strWavConfirm, "wavConfirm");
	REGISTER_STRING(JClientApp::jpApp->strWavPrivateline, "wavPrivateline");
	REGISTER_STRING(JClientApp::jpApp->strWavTopic, "wavTopic");
	REGISTER_STRING(JClientApp::jpApp->strWavJoin, "wavJoin");
	REGISTER_STRING(JClientApp::jpApp->strWavPart, "wavPart");
	REGISTER_STRING(JClientApp::jpApp->strWavPrivate, "wavPrivate");
	REGISTER_STRING(JClientApp::jpApp->strWavAlert, "wavAlert");
	REGISTER_STRING(JClientApp::jpApp->strWavMessage, "wavMessage");
	REGISTER_STRING(JClientApp::jpApp->strWavBeep, "wavBeep");
	REGISTER_STRING(JClientApp::jpApp->strWavClipboard, "wavClipboard");
	ASSERT(lua_gettop(luaVM) == 0);
	return 0;
}

int JClient::lua_getVars(lua_State *luaVM)
{
	lua_getglobal(luaVM, "bReconnect");
	lua_getglobal(luaVM, "bSendByEnter");
	lua_getglobal(luaVM, "bCheatAnonymous");
	int i = 0;
	m_bReconnect = lua_toboolean(luaVM, i++) != 0;
	m_bSendByEnter = lua_toboolean(luaVM, i++) != 0;
	m_bCheatAnonymous = lua_toboolean(luaVM, i++) != 0;
	lua_pop(luaVM, i);
	ASSERT(lua_gettop(luaVM) == 0);
	return 0;
}

int JClient::lua_setVars(lua_State *luaVM)
{
	lua_pushboolean(luaVM, m_bReconnect), lua_setglobal(luaVM, "bReconnect");
	lua_pushboolean(luaVM, m_bSendByEnter), lua_setglobal(luaVM, "bSendByEnter");
	lua_pushboolean(luaVM, m_bCheatAnonymous), lua_setglobal(luaVM, "bCheatAnonymous");
	ASSERT(lua_gettop(luaVM) == 0);
	return 0;
}

int JClient::lua_PlaySound(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 1);
	PlaySound(ANSIToTstr(luaL_checkstring(luaVM, 1)).c_str());
	return 0;
}

int JClient::lua_ShowTopic(lua_State *luaVM)
{
	ShowTopic(lua_isstring(luaVM, 1) ? ANSIToTstr(lua_tostring(luaVM, 1)).c_str() : TEXT(""));
	return 0;
}

int JClient::lua_saveAutoopen(lua_State *luaVM)
{
	saveAutoopen();
	return 0;
}

int JClient::lua_openAutoopen(lua_State *luaVM)
{
	openAutoopen();
	return 0;
}

int JClient::lua_Log(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 1);
	if (lua_isstring(luaVM, 1)) {
		EvLog(ANSIToTstr(lua_tostring(luaVM, 1)), true);
	} else {
		lua_pushstring(luaVM, "incorrect string argument in function \"Log\"");
		lua_error(luaVM);
	}
	return 0;
}

int JClient::lua_HideBaloon(lua_State *luaVM)
{
	HideBaloon();
	return 0;
}

int JClient::lua_Connect(lua_State *luaVM)
{
	bool getsetting = lua_isboolean(luaVM, 1) ? lua_toboolean(luaVM, 1) != 0 : false;
	Connect(getsetting);
	return 0;
}

int JClient::lua_Disconnect(lua_State *luaVM)
{
	DeleteLink(m_clientsock);
	return 0;
}

int JClient::lua_getConnectCount(lua_State *luaVM)
{
	lua_pushinteger(luaVM, m_nConnectCount);
	return 1;
}

int JClient::lua_setConnectCount(lua_State *luaVM)
{
	m_nConnectCount = lua_isnumber(luaVM, 1) ? (int)lua_tointeger(luaVM, 1) : 0;
	return 0;
}

int JClient::lua_getSocket(lua_State *luaVM)
{
	lua_pushinteger(luaVM, m_clientsock);
	return 1;
}

int JClient::lua_checkConnectionButton(lua_State *luaVM)
{
	if (jpPageServer->hwndPage) {
		int check = lua_isboolean(luaVM, 1) ? lua_toboolean(luaVM, 1) : 0;
		CheckDlgButton(jpPageServer->hwndPage, IDC_CONNECT, check);
	}
	return 0;
}

int JClient::lua_WaitConnectStart(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 1);
	int wait = lua_isnumber(luaVM, 1) ? (int)lua_tointeger(luaVM, 1) : 30*1000;
	SetTimer(m_hwndPage, IDT_CONNECT, wait, 0);
	return 0;
}

int JClient::lua_WaitConnectStop(lua_State *luaVM)
{
	KillTimer(hwndPage, IDT_CONNECT);
	return 0;
}

int JClient::lua_MinimizeWindow(lua_State *luaVM)
{
	ShowWindow(m_hwndPage, SW_MINIMIZE);
	return 0;
}

int JClient::lua_MaximizeWindow(lua_State *luaVM)
{
	ShowWindow(m_hwndPage, SW_MAXIMIZE);
	return 0;
}

int JClient::lua_RestoreWindow(lua_State *luaVM)
{
	ShowWindow(m_hwndPage, SW_RESTORE);
	return 0;
}

int JClient::lua_FlashWindow(lua_State *luaVM)
{
	BOOL flash = lua_isboolean(luaVM, 1) ? (BOOL)lua_toboolean(luaVM, 1) : TRUE;
	FlashWindow(m_hwndPage, flash);
	return 0;
}

int JClient::lua_DestroyWindow(lua_State *luaVM)
{
	DestroyWindow(m_hwndPage);
	return 0;
}

int JClient::lua_PageEnable(lua_State *luaVM)
{
	DWORD id;
	if (lua_isstring(luaVM, 1)) id = dCRC(ANSIToTstr(lua_tostring(luaVM, 1)).c_str());
	else if (lua_isnumber(luaVM, 1)) id = (DWORD)lua_tointeger(luaVM, 1);
	else id = CRC_SERVER;
	JPtr<JPage> jp = getPage(id);
	if (jp) {
		jp->Enable();
	}
	return 0;
}

int JClient::lua_PageDisable(lua_State *luaVM)
{
	DWORD id;
	if (lua_isstring(luaVM, 1)) id = dCRC(ANSIToTstr(lua_tostring(luaVM, 1)).c_str());
	else if (lua_isnumber(luaVM, 1)) id = (DWORD)lua_tointeger(luaVM, 1);
	else id = CRC_SERVER;
	JPtr<JPage> jp = getPage(id);
	if (jp) {
		jp->Disable();
	}
	return 0;
}

int JClient::lua_PageAppendScript(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 2);
	DWORD id;
	if (lua_isstring(luaVM, 1)) id = dCRC(ANSIToTstr(lua_tostring(luaVM, 1)).c_str());
	else if (lua_isnumber(luaVM, 1)) id = (DWORD)lua_tointeger(luaVM, 1);
	else id = CRC_SERVER;
	JPtr<JPageLog> jp = getPageLog(id);
	if (jp && lua_isstring(luaVM, 2)) {
		jp->AppendScript(ANSIToTstr(lua_tostring(luaVM, 2)));
	}
	return 0;
}

int JClient::lua_Say(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 2);
	DWORD id;
	if (lua_isstring(luaVM, 1)) id = dCRC(ANSIToTstr(lua_tostring(luaVM, 1)).c_str());
	else if (lua_isnumber(luaVM, 1)) id = (DWORD)lua_tointeger(luaVM, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		Send_Cmd_SAY(m_clientsock, id, SF_TEXT, lua_tostring(luaVM, 2));
	}
	return 0;
}

int JClient::lua_Message(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 2);
	DWORD id;
	if (lua_isstring(luaVM, 1)) id = dCRC(ANSIToTstr(lua_tostring(luaVM, 1)).c_str());
	else if (lua_isnumber(luaVM, 1)) id = (DWORD)lua_tointeger(luaVM, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		Send_Quest_MESSAGE(m_clientsock, id, lua_tostring(luaVM, 2), false, GetSysColor(COLOR_WINDOW));
	}
	return 0;
}

int JClient::lua_Alert(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 2);
	DWORD id;
	if (lua_isstring(luaVM, 1)) id = dCRC(ANSIToTstr(lua_tostring(luaVM, 1)).c_str());
	else if (lua_isnumber(luaVM, 1)) id = (DWORD)lua_tointeger(luaVM, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		Send_Quest_MESSAGE(m_clientsock, id, lua_tostring(luaVM, 2), true, GetSysColor(COLOR_WINDOW));
	}
	return 0;
}

int JClient::lua_Beep(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 1);
	DWORD id;
	if (lua_isstring(luaVM, 1)) id = dCRC(ANSIToTstr(lua_tostring(luaVM, 1)).c_str());
	else if (lua_isnumber(luaVM, 1)) id = (DWORD)lua_tointeger(luaVM, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		Send_Cmd_BEEP(m_clientsock, id);
	}
	return 0;
}

//-----------------------------------------------------------------------------

// The End.