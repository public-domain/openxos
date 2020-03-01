bc /m apm.prj
if errorlevel 1 goto stop
bc /m reports.prj
if errorlevel 1 goto stop
bc /m emcprc.prj
if errorlevel 1 goto stop
bc /m omni800.prj
if errorlevel 1 goto stop
bc /m esprit.prj
if errorlevel 1 goto stop
bc /m presets.prj
if errorlevel 1 goto stop
bc /m prsprc.prj
if errorlevel 1 goto stop
bc /m as400.prj
if errorlevel 1 goto stop
bc /m mapesi.prj
if errorlevel 1 goto stop
bc /m hrsprc.prj
if errorlevel 1 goto stop
:DONEOK
@ECHO Compile and Link Build Successfull
@PAUSE
GOTO ENDOFBAT
:STOP
@ECHO Compile and Link FAILED--FAILED--FAILED
@PAUSE
:ENDOFBAT
