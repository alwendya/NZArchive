@echo off
:debut
cls
echo *****************************************
echo * Testing script for Explorer extension *
echo ***************************************** 
echo.
echo Registering
regsvr32 /s DLL\NZAAddfiles.32.dll >NUL
regsvr32 /s DLL\NZAExtract.32.dll >NUL
echo Registered
TIMEOUT /T 2 /NOBREAK >NUL
echo Hit a key when testing done (Hit Ctrl+C to fully exit) 
pause >NUL
echo Unregistering
regsvr32 /u /s DLL\NZAAddfiles.32.dll >NUL
regsvr32 /u /s DLL\NZAExtract.32.dll >NUL
echo Unregistered
TIMEOUT /T 2 /NOBREAK >NUL
echo Killing explorer.exe
Taskkill /F /IM explorer.exe
echo Explorer killed
TIMEOUT /T 2 /NOBREAK >NUL
echo restarting Explorer
start explorer.exe
echo Launched
TIMEOUT /T 2 /NOBREAK >NUL
echo Re opening current path
explorer.exe %~dp0
echo Re opened
TIMEOUT /T 2 /NOBREAK >NUL
echo Hit a key when new dll is compiled (Hit Ctrl+C to fully exit) 
pause >NUL
goto debut