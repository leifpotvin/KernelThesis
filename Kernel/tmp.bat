@echo off

setlocal EnableDelayedExpansion

set p=%PATH%

:top

for /F "tokens=1,* delims=;" %%a in ("%p%") do (call :func "%%a" & set p=%%b)

if NOT ["%p%"]== [""] goto :top


goto :eof

:func
	if NOT exist %1 echo %~1
	goto :eof