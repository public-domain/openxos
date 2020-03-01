@echo off
batopt ccp=off extend=on
:foo
echo looping!
goto foo
