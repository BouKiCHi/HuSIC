@ECHO OFF

SET FILEDIR=%~p1
SET FILE=%~n1
SET FIN=%FILEDIR%%FILE%
SET FOUT=%FILE%

SET EXECDIR=%~p0

SET TOP_DIR=%EXECDIR%..
SET PCE_INCLUDE=%TOP_DIR%\;%TOP_DIR%\hescode\;%TOP_DIR%\include\pce

"%TOP_DIR%\bin\hmckc" -i -u %* 

if not exist "%FIN%.h" goto end

"%TOP_DIR%\bin\pceas" -raw makepce.s

copy makepce.pce "%FOUT%.pce"

del makepce.pce
del makepce.sym
del makepce.lst
del define.inc
del effect.h
del "%FIN%.h"

goto end

:debug_end
@echo DEBUG mode

:end
