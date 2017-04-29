..\bin\xpcm tone_orig.wav -o tone.pd4
..\bin\xpcm --5bit tone_orig.wav -o tone.pd5

rmdir /S /Q output
mkdir output

call ..\songs\make_hes.bat test01.mml
call ..\songs\make_hes.bat test02.mml
call ..\songs\make_hes.bat test02_5b.mml
call ..\songs\make_hes.bat test03.mml
call ..\songs\make_hes.bat test04.mml
call ..\songs\make_hes.bat test05.mml

call ..\songs\make_hes_multi.bat multi01.mml multi02.mml multi03.mml
call ..\songs\make_pce_multi.bat multi01.mml multi02.mml multi03.mml

call ..\songs\make_hes.bat fmlfo.mml

mv *.hes output
mv *.pce output