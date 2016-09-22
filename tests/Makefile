#
#
#

MAKEHES = ./../songs/make_hes.sh
XPCM = ./../bin/xpcm
PD4WAV = python pd4wav.py

PCMHES = test02.hes test02_5b.hes

HESFILES = test01.hes test03.hes test04.hes test05.hes $(PCMHES)

PCMFILES = tone.pd4 tone.pd5

WAVFILES = dec_pd4.wav dec_pd5.wav

all: $(PCMFILES) $(HESFILES)

pcm: $(PCMFILES) $(PCMHES)

wav: $(WAVFILES)

hes: $(HESFILES)

clean:
	rm -f $(HESFILES)
	rm -f $(PCMFILES)
	rm -f $(WAVFILES)

%.hes : %.mml
	$(MAKEHES) $<

dec_pd4.wav : tone.pd4
	$(PD4WAV) $< -o $@

dec_pd5.wav : tone.pd5
	$(PD4WAV) $< -5 -o $@ 


tone.pd4 : tone_orig.wav
	$(XPCM) $< -o $@

tone.pd5 : tone_orig.wav
		$(XPCM) $< --5bit -o $@


%.pd4 : %.wav
	$(XPCM) $<
