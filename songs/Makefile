
HES = bank01.hes
HES += sample.hes
HES += sample_te.hes
HES += sample_port.hes
HES += sample_slar.hes
HES += sample_mv.hes
HES += sample_panenv.hes

all : $(HES)

clean:
	rm -f *.hes *.pce
	rm -f *.lst *.sym *.h *.inc

%.hes : %.mml
	cmd //c make_hes $<
