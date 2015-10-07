@ECHO OFF

SET FILEDIR=%~p1
SET FILE=%~n1
SET FIN=%FILEDIR%%FILE%
SET FOUT=%FILE%

SET EXECDIR=%~p0


@ECHO %EXECDIR%

SET TOP_DIR=%EXECDIR%..
SET PCE_INCLUDE=%TOP_DIR%\;%TOP_DIR%\hescode\;%TOP_DIR%\include\pce

"%TOP_DIR%\bin\huc" ldr.c
"%TOP_DIR%\bin\pceas" -scd -over ldr.s
"%TOP_DIR%\bin\huc"  main_ovl.c
"%TOP_DIR%\bin\pceas" -scd -over main_ovl.s
"%TOP_DIR%\bin\isolink" c_test.iso ldr.ovl main_ovl.ovl

@REM "%TOP_DIR%\bin\pceas" -raw c_test_hdr.s
@REM "%TOP_DIR%\bin\pceas" -scd c_test_hdr.s
pause
