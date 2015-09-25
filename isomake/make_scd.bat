@ECHO OFF

SET FILEDIR=%~p1
SET FILE=%~n1
SET FIN=%FILEDIR%%FILE%
SET FOUT=%FILE%

SET EXECDIR=%~p0

SET TOP_DIR=%EXECDIR%..
SET PCE_INCLUDE=%TOP_DIR%\;%TOP_DIR%\hescode\;%TOP_DIR%\include\pce

"%TOP_DIR%\bin\hmckc" -i "%FIN%.mml"

if not exist "%FIN%.h" goto end

"%TOP_DIR%\bin\pceas" -scd makepce.s

copy makepce.iso "husic.iso"
copy "%EXECDIR%\iso.cue" "husic.cue"
copy "%EXECDIR%\iso_wav.cue" "husic_iw.cue"

del makepce.iso
del makepce.sym
del makepce.lst
del define.inc
del effect.h
del "%FIN%.h"
