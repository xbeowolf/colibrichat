rem %1 target dir, %2 project dir

if not exist "%1sounds" (
	mkdir "%1sounds"
	xcopy /s "%2sounds" "%1sounds"
)

rem http://www.rsdn.ru/article/winshell/batanyca.xml