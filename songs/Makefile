
HES = bank01.hes
HES += sample.hes
HES += sample_te.hes
HES += sample_port.hes
HES += sample_slar.hes

all : $(HES)

clean:
	rm -f *.hes *.pce

%.hes : %.mml
	cmd //c make_hes $<
