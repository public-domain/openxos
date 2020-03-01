@echo off
echo This is the beginning
xosbatch on ccpoff
dir %1.bat
call baz1 %1
echo got back from master
return
