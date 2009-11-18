-------------------------------------------------------------------------------
-- Startup code and initalization
-------------------------------------------------------------------------------

-- register application functions
chat:regFuncs();
-- get application setting: sounds, metrics, register setting.
chat:getGlobal();
-- auto open channels only once at program starting
fAutoopen = true;

-------------------------------------------------------------------------------
-- Network transactions
-------------------------------------------------------------------------------

-- Connection established
function onLinkConnect()
	chat:setVars(); -- from host to script
	nConnectCount = 0;
	chat:getVars(); -- from script to host
end

-- Success identification after established connection
function onLinkStart()
	if fAutoopen then
		chat:openAutoopen();
		fAutoopen = false;
	end
end

-------------------------------------------------------------------------------
-- Windows messages
-------------------------------------------------------------------------------

-- WM_CREATE
function wmCreate()
	if profile.getInt("Client\\", "ConnectionState", 0) ~= 0 then
		chat:Connect(false);
	else
		chat:Log("[style=msg]Ready to connect[/style]");
	end
end

-- WM_DESTROY
function wmDestroy()
	profile.setInt("Client\\", "ConnectionState", chat:getSocket());
	chat:saveAutoopen();

	if chat:getSocket() ~= 0 then
		chat:Disconnect();
	end
end

-- WM_ENTERSIZEMOVE
function wmEnterSizeMove()
	chat:HideBaloon();
end

-- WM_ACTIVATEAPP
function wmActivateApp(activated)
	chat:HideBaloon();
end

-- WM_COMMAND preprocess call
function wmCommand(id)
	chat:HideBaloon();
end

-- The End.