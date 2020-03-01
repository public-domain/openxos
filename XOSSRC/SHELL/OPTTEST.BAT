@echo off
REM Test BATOPT keyword
ECHO * Test BATOPT with no options
batopt
ECHO *
ECHO * Test BATOPT with EXTEND option
batopt extend
batopt/quiet extend
if errorlevel 1 echo errorlevel shows extend is on
if not errorlevel 1 echo errorlevel shows extend is off
batopt extend=on
batopt extend
batopt extend=off
batopt extend
ECHO *
ECHO * Test BATOPT with CCP option
batopt ccp
batopt/quiet ccp
if errorlevel 1 echo errorlevel shows ccp is on
if not errorlevel 1 echo errorlevel shows ccp is off
batopt ccp=on
batopt ccp
batopt ccp=off
batopt ccp
ECHO *
ECHO * Test BATOPT with ECHO option
batopt echo
batopt/quiet echo
if errorlevel 1 echo errorlevel shows echo is on
if not errorlevel 1 echo errorlevel shows echo is off
batopt echo=on
batopt echo
if errorlevel 1 echo errorlevel shows echo is on
if not errorlevel 1 echo errorlevel shows echo is off
batopt echo=off
batopt echo

