@echo off
:NEXT
IF ".%1"=="." GOTO EXIT
ECHO %1
SHIFT
GOTO NEXT

:EXIT
