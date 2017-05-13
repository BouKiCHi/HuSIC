#
#
#

MAKEHES =  ..\songs\make_hes.bat
MAKEPCE =  ..\songs\make_pce.bat
MAKEHES_MULTI =  ..\songs\make_hes_multi.bat
MAKEPCE_MULTI =  ..\songs\make_pce_multi.bat

# MAKEHES = ./../songs/make_hes.sh
XPCM = ..\bin\xpcm
PD4WAV = python pd4wav.py

PCMHES = test02.hes test02_5b.hes

HESFILES = test01.hes test03.hes test04.hes test05.hes $(PCMHES) fmlfo.hes multi01.hes

PCEFILES = multi01.pce

PCMFILES = tone.pd4 tone.pd8

WAVFILES = dec_pd4.wav

OUTPUT = output

all: output_dir $(PCMFILES) $(HESFILES) $(PCEFILES)

output_dir:
	rm -rf $(OUTPUT)
	mkdir $(OUTPUT)

pcm: $(PCMFILES) $(PCMHES)

wav: $(WAVFILES)

hes: $(HESFILES)

clean:
	rm -f $(HESFILES)
	rm -f $(PCMFILES)
	rm -f $(WAVFILES)


multi01.hes : 
	$(MAKEHES_MULTI) multi01.mml multi02.mml multi03.mml 
	mv $@ output

multi01.pce : 
	$(MAKEPCE_MULTI) multi01.mml multi02.mml multi03.mml 
	mv $@ output


%.hes : %.mml
	$(MAKEHES) $< 
	mv $@ output

%.pce : %.mml
	$(MAKEPCE) $< 
	mv $@ output

dec_pd4.wav : tone.pd4
	$(PD4WAV) $< -o $@

dec_pd5.wav : tone.pd5
	$(PD4WAV) $< -5 -o $@ 


tone.pd4 : tone_orig.wav
	$(XPCM) $< -o $@

tone.pd8 : tone_orig.wav
		$(XPCM) $< --5bit -o $@

%.pd4 : %.wav
	$(XPCM) $<
