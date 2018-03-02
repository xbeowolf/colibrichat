---UTF8------------------------------------------------------------------------
--                                ATTENTION!                                 --
-- Developer provides this script as is with no warranty of proper work on   --
-- any users modifications. All modifications are carried out at own risk.   --
--                   © BEOWOLF / Podobashev Dmitry, 2009                     --
--     email: xbeowolf@gmail.com     skype: x-beowulf     icq: 320329575     --
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Startup code and initialization
-------------------------------------------------------------------------------

-- Windows register folders
RF_CLIENT = "Client\\"
RF_AUTOOPEN = "Client\\Autoopen\\"
RF_SOUNDS = "Sounds\\"

-- Reserved contacts identifiers
NAME_SERVER = "Server"
NAME_LIST = "Channels"
NAME_NONAME = "Noname"
NAME_ANONYMOUS = "Anonymous"

-- auto opening channels only once at program start
local Autoopen = true
-- user nickname
local nickOwn = NAME_NONAME
-- chat state
local Reconnect = true
local SendByEnter = true
local CheatAnonymous = false

-- WSA error codes
local wsaErr = {
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
	[3] = "The connection reset because IP-address is banned.", -- WSABANNED
	[10050] = "The network subsystem failed.", -- WSAENETDOWN
	[10054] = "The connection was reset by the remote side.", -- WSAECONNRESET
	[10053] = "The connection was terminated due to a time-out or other failure.", -- WSAECONNABORTED
}

-- Logging setting

elogDef   = 0
elogError = 1
elogWarn  = 2
elogIgnor = 3
elogInfo  = 4
elogMsg   = 5
elogDescr = 6
elogItrn  = 7
elogOtrn  = 8
-- set value to true to enable that type of logging, or set value to false to disable it.
LogSet = {
	[elogDef  ] = true,
	[elogError] = true,
	[elogWarn ] = true,
	[elogIgnor] = true,
	[elogInfo ] = true,
	[elogMsg  ] = true,
	[elogDescr] = true,
	[elogItrn ] = true,
	[elogOtrn ] = true,
}
-- Alert icon indexes
eGreen  = 0
eBlue   = 1
eYellow = 2
eRed    = 3

-- Wave sounds
local wave = {
	meline = profile.getStr(RF_SOUNDS,"MeLine","Sounds\\me_line.wav"),
	chatline = profile.getStr(RF_SOUNDS,"ChatLine","Sounds\\chat_line.wav"),
	confirm = profile.getStr(RF_SOUNDS,"Confirm","Sounds\\confirm.wav"),
	privateline = profile.getStr(RF_SOUNDS,"PrivateLine","Sounds\\chat_line.wav"),
	topic = profile.getStr(RF_SOUNDS,"Topic","Sounds\\topic_change.wav"),
	join = profile.getStr(RF_SOUNDS,"Join","Sounds\\channel_join.wav"),
	part = profile.getStr(RF_SOUNDS,"Part","Sounds\\channel_leave.wav"),
	private = profile.getStr(RF_SOUNDS,"Private","Sounds\\private_start.wav"),
	alert = profile.getStr(RF_SOUNDS,"Alert","Sounds\\alert.wav"),
	message = profile.getStr(RF_SOUNDS,"Message","Sounds\\message.wav"),
	beep = profile.getStr(RF_SOUNDS,"Beep","Sounds\\beep.wav"),
	clipboard = profile.getStr(RF_SOUNDS,"Clipboard","Sounds\\clipboard.wav"),
}

Alert = {
	Ready = {
		fFlashPageNew = true,
		fFlashPageSayPrivate = true,
		fFlahPageSayChannel = true,
		fFlashPageChangeTopic = true,
		fCanOpenPrivate = true,
		fCanAlert = true,
		fCanMessage = true,
		fCanSplash = true,
		fCanSignal = true,
		fCanRecvClipboard = true,
		fPlayChatSounds = true,
		fPlayPrivateSounds = true,
		fPlayAlert = true,
		fPlayMessage = true,
		fPlayBeep = true,
		fPlayClipboard = true,
	},
	DND = {
		fFlashPageNew = false,
		fFlashPageSayPrivate = false,
		fFlahPageSayChannel = false,
		fFlashPageChangeTopic = false,
		fCanOpenPrivate = false,
		fCanAlert = false,
		fCanMessage = false,
		fCanSplash = false,
		fCanSignal = false,
		fCanRecvClipboard = false,
		fPlayChatSounds = false,
		fPlayPrivateSounds = false,
		fPlayAlert = false,
		fPlayMessage = false,
		fPlayBeep = false,
		fPlayClipboard = false,
	},
	Busy = {
		fFlashPageNew = false,
		fFlashPageSayPrivate = true,
		fFlahPageSayChannel = false,
		fFlashPageChangeTopic = false,
		fCanOpenPrivate = true,
		fCanAlert = true,
		fCanMessage = true,
		fCanSplash = false,
		fCanSignal = true,
		fCanRecvClipboard = false,
		fPlayChatSounds = false,
		fPlayPrivateSounds = true,
		fPlayAlert = true,
		fPlayMessage = false,
		fPlayBeep = true,
		fPlayClipboard = true,
	},
	NA = {
		fFlashPageNew = false,
		fFlashPageSayPrivate = false,
		fFlahPageSayChannel = false,
		fFlashPageChangeTopic = false,
		fCanOpenPrivate = false,
		fCanAlert = true,
		fCanMessage = false,
		fCanSplash = false,
		fCanSignal = true,
		fCanRecvClipboard = true,
		fPlayChatSounds = false,
		fPlayPrivateSounds = true,
		fPlayAlert = true,
		fPlayMessage = false,
		fPlayBeep = true,
		fPlayClipboard = true,
	},
	Away = {
		fFlashPageNew = true,
		fFlashPageSayPrivate = true,
		fFlahPageSayChannel = false,
		fFlashPageChangeTopic = false,
		fCanOpenPrivate = true,
		fCanAlert = true,
		fCanMessage = true,
		fCanSplash = true,
		fCanSignal = true,
		fCanRecvClipboard = true,
		fPlayChatSounds = false,
		fPlayPrivateSounds = true,
		fPlayAlert = true,
		fPlayMessage = true,
		fPlayBeep = true,
		fPlayClipboard = true,
	},
	Invisible = {
		fFlashPageNew = true,
		fFlashPageSayPrivate = true,
		fFlahPageSayChannel = false,
		fFlashPageChangeTopic = true,
		fCanOpenPrivate = true,
		fCanAlert = true,
		fCanMessage = true,
		fCanSplash = true,
		fCanSignal = true,
		fCanRecvClipboard = true,
		fPlayChatSounds = true,
		fPlayPrivateSounds = true,
		fPlayAlert = true,
		fPlayMessage = true,
		fPlayBeep = true,
		fPlayClipboard = true,
	},
}

-- Lets own accessibility be same as standard
OwnAccess = Alert.Ready

-- Channels access status descriptions
chanStatName = {}
chanStatName[0] = "outsider"
chanStatName[1] = "reader"
chanStatName[2] = "writer"
chanStatName[3] = "member"
chanStatName[4] = "moderator"
chanStatName[5] = "administrator"
chanStatName[6] = "founder"

-- Controls identifiers, those constants defined in program code
-- do not change those values without recompile
idcOk        = 1 -- IDOK
idcCancel    = 2 -- IDCANCEL

-------------------------------------------------------------------------------
-- Helper functions
-------------------------------------------------------------------------------

function colorNick(nick)
	return nick == nickOwn and "blue" or "red"
end

-------------------------------------------------------------------------------
-- Service events
-------------------------------------------------------------------------------

function JClient:onInit()
end

function JClient:onDone()
end

function JClient:onRun()
end

-------------------------------------------------------------------------------
-- Network events
-------------------------------------------------------------------------------

-- Connection established
function JClient:onLinkConnect()
	self:setConnectCount(0)
end

-- Success identification after established connection
function JClient:onLinkStart()
	if Autoopen and profile.getInt(RF_AUTOOPEN,"UseAutoopen",0) ~= 0 then
		self:openAutoopen()
		Autoopen = false
	end
end

-- Connection closed
function JClient:onLinkClose(idErr)
	self:setVars(Reconnect,SendByEnter,CheatAnonymous) -- from host to script
	self:Log("Disconnected. Reason: [i]"..wsaErr[idErr].."[/i]",elogMsg)
	if idErr ~= 0 and Reconnect then
		self:Connect(false) -- if not disconnected by user, try to reconnect again
	else
		self:checkConnectionButton(false)
	end
end

-- Connection failed
function JClient:onLinkFail(idErr)
	self:setVars(Reconnect,SendByEnter,CheatAnonymous) -- from host to script
	self:Log("Connecting failed. Reason: [i]"..wsaErr[idErr].."[/i]",elogMsg)
	if idErr ~= 0 and Reconnect then
		local v = profile.getInt(RF_CLIENT,"TimerConnect",30*1000)
		self:WaitConnectStart(v) -- if not disconnected by user, try to reconnect again
		self:Log("Waiting "..(v/1000).." seconds and try again (attempt #"..self:getConnectCount()..").",elogMsg)
	end
end

function JClient:onLog(str,elog)
	if LogSet[elog] then
		local msg
		if elog == elogDef then
			msg = "[style=Default]"..str.."[/style]"
		elseif elog == elogError then
			msg = "[style=Error]"..str.."[/style]"
			self:PageSetIcon(NAME_SERVER,eRed)
		elseif elog == elogWarn then
			msg = "[style=Warning]"..str.."[/style]"
			self:PageSetIcon(NAME_SERVER,eYellow)
		elseif elog == elogInfo then
			msg = "[style=Info]"..str.."[/style]"
			self:PageSetIcon(NAME_SERVER,eBlue)
		elseif elog == elogMsg then
			msg = "[style=Msg]"..str.."[/style]"
			self:PageSetIcon(NAME_SERVER,eBlue)
		elseif elog == elogDescr then
			msg = "[style=Descr]"..str.."[/style]"
			self:PageSetIcon(NAME_SERVER,eBlue)
		elseif elog == elogItrn then
			msg = str
		elseif elog == elogOtrn then
			msg = str
		else
			msg = str
		end
		self:PageAppendScript(NAME_SERVER,"[style=time]"..os.date("[%X] ").."[/style]"..msg)
	end
end

-------------------------------------------------------------------------------
-- Transactions
-------------------------------------------------------------------------------

-- User has changed own nickname
function JClient:onNickOwn(nickNew)
	nickOwn = nickNew
end

-- Contacted user or yourself has changed nickname
function JClient:onNick(nickOld,nickNew)
end

-- User has joined to channel
function JClient:onJoinChannel(nickWho,channel)
	self:PageAppendScript(channel,"[style=Info]joins: [b]"..nickWho.."[/b][/style]")
end

-- You opens channel
function JClient:onOpenChannel(channel)
	self:PageAppendScript(channel,"[style=Info]now talking in [b]#"..channel.."[/b][/style]")
end

-- User has parted from channel
function JClient:onPartChannel(nickWho,nickBy,channel)
	if nickWho == nickBy then
		self:PageAppendScript(channel,"[style=Info]parts: [b]"..nickWho.."[/b][/style]")
	elseif nickBy == NAME_SERVER then
		self:PageAppendScript(channel,"[style=Info]quits: [b]"..nickWho.."[/b][/style]")
	else
		self:PageAppendScript(channel,"[style=Info][b]"..nickWho.."[/b] was kicked by [b]"..nickBy.."[/b][/style]")
	end
end

-- nickWho opened private talk with you
function JClient:onJoinPrivate(nickWho)
	self:PageAppendScript(nickWho,"[style=Info][b]"..nickWho.."[/b] call you to private talk[/style]")
	if OwnAccess.fPlayMessage then self:PlaySound(wave.private) end
end

-- You opens private talk with nickWho
function JClient:onOpenPrivate(nickWho)
	self:PageAppendScript(nickWho,"[style=Info]now talk with [b]"..nickWho.."[/b][/style]")
end

-- Closes private talk with nickWho
function JClient:onPartPrivate(nickWho,nickBy)
	if nickWho == nickBy then
		self:PageAppendScript(nickWho,"[style=Info][b]"..nickWho.."[/b] leave private talk[/style]")
	elseif nickBy == NAME_SERVER then
		self:PageAppendScript(nickWho,"[style=Info][b]"..nickWho.."[/b] was disconnected[/style]")
	else
		self:PageAppendScript(nickWho,"[style=Info][b]"..nickWho.."[/b] was kicked by [b]"..nickBy.."[/b][/style]")
	end
end

-- Online user status, "online" value can be: 0 - offline, 1 - online, 2 - typing
function JClient:onOnline(nickWho,online)
end

-- User changes own status, "status" value can be: 0 - ready, 1 - DND, 2 - Busy, 3 - N/A, 4 - Away, 5 - invisible
function JClient:onStatusMode(nickWho,status,alert)
	if nickWho == nickOwn then
		OwnAccess = alert
	end
end

-- User changes status image, "imgIndex" represent image index
function JClient:onStatusImage(nickWho,imgIndex)
end

-- User changes status message, "msg" is string value
function JClient:onStatusMessage(nickWho,msg)
end

-- User changes "god" cheat status, boolean value
function JClient:onStatusGod(nickWho,god)
	self:PageAppendScript(NAME_SERVER,god and nickWho.." become a god" or nickWho.." ceased to be a god")
end

-- User changes "devil" cheat status, boolean value
function JClient:onStatusDevil(nickWho,devil)
	self:PageAppendScript(NAME_SERVER,devil and nickWho.." become a devil" or nickWho.." ceased to be a devil")
end

-- User said "text" at "where"
function JClient:onSay(nickWho,where,text)
	self:PageAppendScript(where,"[color="..colorNick(nickWho).."]"..nickWho..os.date(" (%c)").."[/color]")
	local plain = self:PageAppendRtf(where,text)

	if str.hasstr(plain,"Hello, "..nickOwn) > 0 then
		self:Say(where,"Hi, "..nickWho.."\r\n")
	elseif str.hasstr(plain,"Fuck you, "..nickOwn) > 0 then
		self:Beep(nickWho)
	end
end

-- Channel topic was changed by nickWho
function JClient:onTopic(nickWho,where,topic)
	self:PageAppendScript(where,"[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changes topic to:\n[u]"..topic.."[/u][/style]")
	if OwnAccess.fPlayChatSounds then self:PlaySound(wave.topic) end
end

-- Channel "autostatus" value was changed by nickWho
-- "autostatus" can be: 0 - outsider, 1 - reader, 2 - writer, 3 - member, 4 - moderator, 5 - admin, 6 - founder
function JClient:onChanAutostatus(nickWho,where,autostatus)
	self:PageAppendScript(where,"[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changes channel users entry status to [b]"..chanStatName[autostatus].."[/b][/style]")
	if OwnAccess.fPlayChatSounds then self:PlaySound(wave.topic) end
end

-- Channel "limit" integer value was changed by nickWho
function JClient:onChanLimit(nickWho,where,limit)
	self:PageAppendScript(where,"[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel limit to "..limit.." users[/style]")
	if OwnAccess.fPlayChatSounds then self:PlaySound(wave.topic) end
end

-- Channel "hidden" boolean state was changed by nickWho
function JClient:onChanHidden(nickWho,where,hidden)
	self:PageAppendScript(where,hidden
		and "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as hidden[/style]"
		or  "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as visible[/style]")
	if OwnAccess.fPlayChatSounds then self:PlaySound(wave.topic) end
end

-- Channel "anonymous" boolean state was changed by nickWho
function JClient:onChanAnonymous(nickWho,where,anonymous)
	self:PageAppendScript(where,anonymous
		and "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as anonymous[/style]"
		or  "[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] sets channel as not anonymous[/style]")
	if OwnAccess.fPlayChatSounds then self:PlaySound(wave.topic) end
end

-- Page sheet color at "where" was changed by nickWho
-- red, green, blue values indicate RGB-color at range 0-255
function JClient:onBackground(nickWho,where,red,green,blue)
	self:PageAppendScript(where,"[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changes sheet color[/style]")
	if OwnAccess.fPlayChatSounds then self:PlaySound(wave.topic) end
end

-- User channel status on "where" was changed by "nickBy"
-- "status" can be: 0 - outsider, 1 - reader, 2 - writer, 3 - member, 4 - moderator, 5 - admin, 6 - founder
function JClient:onAccess(nickWho,where,status,nickBy)
	self:PageAppendScript(where,"[style=Descr][color="..colorNick(nickWho).."]"..nickWho.."[/color] changed channel status to [b]"..chanStatName[status].."[/b] by [color="..colorNick(nickBy).."]"..nickBy.."[/color][/style]")
end

-- User sends sound signal
function JClient:onBeep(nickWho)
	if OwnAccess.fPlayBeep then self:PlaySound(wave.beep) end
	--self:Alert(nickWho, "Yes, I'm listen")
	self:Log("sound signal from [b]"..nickWho.."[/b]",elogInfo)
end

-- User sends Windows clipboard content
function JClient:onClipboard(nickWho)
	if OwnAccess.fPlayClipboard then self:PlaySound(wave.clipboard) end
	--self:Message(nickWho, "Thank you")
end

-------------------------------------------------------------------------------
-- Windows messages
-------------------------------------------------------------------------------

-- WM_CREATE
function JClient:wmCreate()
	if profile.getInt(RF_CLIENT,"ConnectionState",0) ~= 0 then
		self:Connect(false)
	else
		self:Log("Ready to connect",elogMsg)
	end
end

-- WM_DESTROY
function JClient:wmDestroy()
	profile.setInt(RF_CLIENT,"ConnectionState",self:getSocket())
	if not Autoopen then
		self:saveAutoopen() -- save the state of channels only if they were opened
	end

	if self:getSocket() ~= 0 then
		self:Disconnect()
	end
end

-- WM_CLOSE
function JClient:wmClose()
	self:DestroyWindow()
end

-- WM_ENTERSIZEMOVE
function JClient:wmEnterSizeMove()
	self:BaloonHide()
end

-- WM_ACTIVATEAPP
function JClient:wmActivateApp(activated)
	self:BaloonHide()
end

-- WM_COMMAND preprocess call
function JClient:wmCommand(id)
	self:BaloonHide()
	if id == idcCancel then -- IDCANCEL for main window
		self:wmClose()
		--self:MinimizeWindow()
	end
end

-------------------------------------------------------------------------------
-- Commands response
-------------------------------------------------------------------------------

-- IDC_CONNECT on "Server" page
function JClient:idcConnect()
	if self:getSocket() ~= 0 then
		self:Disconnect()
	elseif self:getConnectCount() ~= 0 then
		self:setConnectCount(0)
		self:WaitConnectStop()
		self:checkConnectionButton(false);
		self:PageDisable(NAME_SERVER)
		self:Log("Canceled.",elogMsg)
	else
		self:Connect(true)
	end
end

-- The End.