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
SONGFILES = $(HESFILES) $(PCEFILES)
MULTI_MMLFILES = multi01.mml multi02.mml multi03.mml 

PCMFILES = tone.pd4 tone.pd8

WAVFILES = dec_pd4.wav

OUTPUT = output

all: output_dir $(PCMFILES) $(addprefix $(OUTPUT)/,$(SONGFILES))

# all: output_dir $(OUTPUT)/test01.hes

output_dir:
	mkdir -p $(OUTPUT)

pcm: $(PCMFILES) $(PCMHES)

wav: $(WAVFILES)

hes: $(HESFILES)

clean:
	rm -rf $(OUTPUT)
	rm -f $(WAVFILES)

$(OUTPUT)/multi01.hes : $(MULTI_MMLFILES)
	$(MAKEHES_MULTI) $^
	@mv $(notdir $@) $@ 

$(OUTPUT)/multi01.pce : $(MULTI_MMLFILES) 
	$(MAKEPCE_MULTI) $^
	@mv $(notdir $@) $@ 


$(OUTPUT)/%.hes : ../hescode/main.s
	$(MAKEHES) $(@F:.hes=.mml)
	@mv $(notdir $@) $@ 

$(OUTPUT)/%.pce : ../hescode/main.s
	$(MAKEPCE) $(@F:.pce=.mml)
	@mv $(notdir $@) $@ 

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

.PHONY: all output_dir clean
