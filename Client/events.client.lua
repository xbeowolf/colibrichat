-------------------------------------------------------------------------------
--                                ATTENTION!                                 --
-- Developer provides this script as is with no warranty of proper work on   --
-- any users modifications. All modifications are carried out at own risk.   --
--                   © BEOWOLF / Podobashev Dmitry, 2009                     --
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Startup code and initialization
-------------------------------------------------------------------------------

-- Windows register folders
RF_CLIENT = "Client\\"
RF_AUTOOPEN = "Client\\Autoopen\\"
RF_SOUNDS = "Client\\Sounds\\"

-- Reserved contacts identifiers
NAME_SERVER = "Server"
NAME_LIST = "Channels"
NAME_NONAME = "Noname"
NAME_ANONYMOUS = "Anonymous"

-- get application setting: sounds, metrics, register setting.
this:getGlobal()
-- auto opening channels only once at program start
fAutoopen = true
-- user nickname
nickOwn = NAME_NONAME

-- WSA error codes
wsaErr = {
	-- on FD_CONNECT
	[10061] = "The attempt to connect was rejected.", -- WSAECONNREFUSED
	[10051] = "The network cannot be reached from this host at this time.", -- WSAENETUNREACH
	[10024] = "No more file descriptors are available.", -- WSAEMFILE
	[10055] = "No buffer space is available. The socket cannot be connected.", -- WSAENOBUFS
	[10057] = "The socket is not connected.", -- WSAENOTCONN
	[10060] = "Attempt to connect timed out without establishing a connection.", -- WSAETIMEDOUT
	-- on FD_CLOSE
	[0] = "The connection was reset by software itself.",
	[1] = "The connection reset by validate timeout.", -- WSAVALIDATETIME
	[2] = "The connection reset because was recieved transaction with bad CRC code", -- WSABADCRC
	[10050] = "The network subsystem failed.", -- WSAENETDOWN
	[10054] = "The connection was reset by the remote side.", -- WSAECONNRESET
	[10053] = "The connection was terminated due to a time-out or other failure.", -- WSAECONNABORTED
}

-- Channels access status descriptions
chanStatName = {}
chanStatName[0] = "outsider"
chanStatName[1] = "reader"
chanStatName[2] = "writer"
chanStatName[3] = "member"
chanStatName[4] = "moderator"
chanStatName[5] = "administrator"
chanStatName[6] = "founder"

-------------------------------------------------------------------------------
-- Helper functions
-------------------------------------------------------------------------------

function colorNick(nick)
	if nick ~= nickOwn then return "red"
	else return "blue"
	end
end

-------------------------------------------------------------------------------
-- Service events
-------------------------------------------------------------------------------

function onInit()
end

function onDone()
end

function onRun()
end

-------------------------------------------------------------------------------
-- Network events
-------------------------------------------------------------------------------

-- Connection established
function onLinkConnect()
	this:setConnectCount(0)
end

-- Success identification after established connection
function onLinkStart()
	if fAutoopen and profile.getInt(RF_AUTOOPEN, "UseAutoopen", 0) ~= 0 then
		this:openAutoopen()
		fAutoopen = false
	end
end

-- Connection closed
function onLinkClose(idErr)
	this:setVars() -- from host to script
	this:Log("[style=msg]Disconnected. Reason: [i]"..wsaErr[idErr].."[/i][/style]")
	if idErr ~= 0 and bReconnect then
		this:Connect(false) -- if not disconnected by user, try to reconnect again
	else
		this:checkConnectionButton(false)
	end
end

-- Connection failed
function onLinkFail(idErr)
	this:setVars() -- from host to script
	this:Log("[style=msg]Connecting failed. Reason: [i]"..wsaErr[idErr].."[/i][/style]")
	if idErr ~= 0 and bReconnect then
		local v = profile.getInt(RF_CLIENT, "TimerConnect", 30*1000)
		this:WaitConnectStart(v) -- if not disconnected by user, try to reconnect again
		this:Log("[style=msg]Waiting "..(v/1000).." seconds and try again (attempt #"..this:getConnectCount()..").[/style]")
	end
end

-------------------------------------------------------------------------------
-- Transactions
-------------------------------------------------------------------------------

-- User has changed own nickname
function onNickOwn(nickNew)
	nickOwn = nickNew
end

-- Contacted user or yourself has changed nickname
function onNick(nickOld, nickNew)
end

-- User has joined to channel
function onJoinChannel(nickWho, channel)
	this:PageAppendScript(channel, "[style=Info]joins: [b]"..nickWho.."[/b][/style]")
end

-- You opens channel
function onOpenChannel(channel)
	this:PageAppendScript(channel, "[style=Info]now talking in [b]#"..channel.."[/b][/style]")
end

-- User has parted from channel
function onPartChannel(nickWho, nickBy, channel)
	if nickWho == nickBy then
		this:PageAppendScript(channel, "[style=Info]parts: [b]"..nickWho.."[/b][/style]")
	elseif nickBy == NAME_SERVER then
		this:PageAppendScript(channel, "[style=Info]quits: [b]"..nickWho.."[/b][/style]")
	else
		this:PageAppendScript(channel, "[style=Info][b]"..nickWho.."[/b] was kicked by [b]"..nickBy.."[/b][/style]")
	end
end

-- nickWho opened private talk with you
function onJoinPrivate(nickWho)
	this:PageAppendScript(nickWho, "[style=Info][b]"..nickWho.."[/b] call you to private talk[/style]")
end

-- You opens private talk with nickWho
function onOpenPrivate(nickWho)
	this:PageAppendScript(nickWho, "[style=Info]now talk with [b]"..nickWho.."[/b][/style]")
end

-- Closes private talk with nickWho
function onPartPrivate(nickWho, nickBy)
	if nickWho == nickBy then
		this:PageAppendScript(nickWho, "[style=Info][b]"..nickWho.."[/b] leave private talk[/style]")
	elseif nickBy == NAME_SERVER then
		this:PageAppendScript(nickWho, "[style=Info][b]"..nickWho.."[/b] was disconnected[/style]")
	else
		this:PageAppendScript(nickWho, "[style=Info][b]"..nickWho.."[/b] was kicked by [b]"..nickBy.."[/b][/style]")
	end
end

-- Online user status, "online" value can be: 0 - offline, 1 - online, 2 - typing
function onOnline(nickWho, online)
end

-- User changes own status, "status" value can be: 0 - ready, 1 - DND, 2 - Busy, 3 - N/A, 4 - Away, 5 - invisible
function onStatusMode(nickWho, status)
end

-- User changes status image, "imgIndex" represent image index
function onStatusImage(nickWho, imgIndex)
end

-- User changes status message, "msg" is string value
function onStatusMessage(nickWho, msg)
end

-- User changes "god" cheat status, boolean value
function onStatusGod(nickWho, god)
	if god then
		this:PageAppendScript(NAME_SERVER, nickWho.." become a god")
	else
		this:PageAppendScript(NAME_SERVER, nickWho.." ceased to be a god")
	end
end

-- User changes "devil" cheat status, boolean value
function onStatusDevil(nickWho, devil)
	if devil then
		this:PageAppendScript(NAME_SERVER, nickWho.." become a devil")
	else
		this:PageAppendScript(NAME_SERVER, nickWho.." ceased to be a devil")
	end
end

-- User said "text" at "where"
function onSay(nickWho, where, text)
	if str.hasstr(text, "Hello, "..nickOwn) > 0 then
		this:Say(where, "Hi, "..nickWho.."\r\n")
	elseif str.hasstr(text, "Fuck you, "..nickOwn) > 0 then
		this:Beep(nickWho)
	end
end

-- Channel topic was changed by nickWho
function onTopic(nickWho, where, topic)
	this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changes topic to:\n[u]"..topic.."[/u][/style]")
end

-- Channel "autostatus" value was changed by nickWho
-- "autostatus" can be: 0 - outsider, 1 - reader, 2 - writer, 3 - member, 4 - moderator, 5 - admin, 6 - founder
function onChanAutostatus(nickWho, where, autostatus)
	this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changes channel users entry status to [b]"..chanStatName[autostatus].."[/b][/style]")
end

-- Channel "limit" integer value was changed by nickWho
function onChanLimit(nickWho, where, limit)
	this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel limit to "..limit.." users[/style]")
end

-- Channel "hidden" boolean state was changed by nickWho
function onChanHidden(nickWho, where, hidden)
	if hidden then
		this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as hidden[/style]")
	else
		this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as visible[/style]")
	end
end

-- Channel "anonymous" boolean state was changed by nickWho
function onChanAnonymous(nickWho, where, anonymous)
	if anonymous then
		this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as anonymous[/style]")
	else
		this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as not anonymous[/style]")
	end
end

-- Page sheet color at "where" was changed by nickWho
-- red, green, blue values indicate RGB-color at range 0-255
function onBackground(nickWho, where, red, green, blue)
	this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changes sheet color[/style]")
end

-- User channel status on "where" was changed by "nickBy"
-- "status" can be: 0 - outsider, 1 - reader, 2 - writer, 3 - member, 4 - moderator, 5 - admin, 6 - founder
function onAccess(nickWho, where, status, nickBy)
	this:PageAppendScript(where, "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changed channel status to [b]"..chanStatName[status].."[/b] by [color="..colorNick(nickBy).."]"..nickBy.."[/color][/style]")
end

-- User sends sound signal
function onBeep(nickWho)
	--this:Alert(nickWho, "Yes, I'm listen")
end

-- User sends Windows clipboard content
function onClipboard(nickWho)
	--this:Message(nickWho, "Thank you")
end

-------------------------------------------------------------------------------
-- Windows messages
-------------------------------------------------------------------------------

-- WM_CREATE
function wmCreate()
	if profile.getInt(RF_CLIENT, "ConnectionState", 0) ~= 0 then
		this:Connect(false)
	else
		this:Log("[style=msg]Ready to connect[/style]")
	end
end

-- WM_DESTROY
function wmDestroy()
	profile.setInt(RF_CLIENT, "ConnectionState", this:getSocket())
	if not fAutoopen then
		this:saveAutoopen() -- save the state of channels only if they were opened
	end

	if this:getSocket() ~= 0 then
		this:Disconnect()
	end
end

-- WM_CLOSE
function wmClose()
	this:DestroyWindow()
end

-- WM_ENTERSIZEMOVE
function wmEnterSizeMove()
	this:HideBaloon()
end

-- WM_ACTIVATEAPP
function wmActivateApp(activated)
	this:HideBaloon()
end

-- WM_COMMAND preprocess call
function wmCommand(id)
	this:HideBaloon()
end

-------------------------------------------------------------------------------
-- Commands response
-------------------------------------------------------------------------------

-- IDCANCEL for main window
function idcCancel()
	wmClose()
	--this:MinimizeWindow()
end

-- IDC_CONNECT on "Server" page
function idcConnect()
	if this:getSocket() ~= 0 then
		this:Disconnect()
	elseif this:getConnectCount() ~= 0 then
		this:setConnectCount(0)
		this:WaitConnectStop()
		this:checkConnectionButton(false);
		this:PageDisable(NAME_SERVER)
		this:Log("[style=msg]Canceled.[/style]")
	else
		this:Connect(true)
	end
end

-- The End.