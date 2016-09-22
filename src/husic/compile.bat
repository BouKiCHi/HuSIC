@ECHO OFF

SET FILEDIR=%~p1
SET FILE=%~n1
SET FIN=%FILEDIR%%FILE%
SET FOUT=%FILE%

SET EXECDIR=%~p0

SET TOP_DIR=%EXECDIR%..\..
SET PCE_INCLUDE=%TOP_DIR%\;%TOP_DIR%\hescode\;%TOP_DIR%\include\pce

"%TOP_DIR%\bin\huc" main.c

@ECHO Copy to hescode
copy main.s "%TOP_DIR%\hescode\"
del main.s

pause
