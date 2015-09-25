#
# Makefile for HuSIC
#

# command arguments:
# make [hmckc|xpcm|huc|husic]
#

DATE = $(shell date +"%y%m%d")
ZIPNAME = husic_$(DATE).zip

ifeq ($(WIN32),1)

PFX = i586-mingw32-
CC = $(PFX)gcc
EXESFX = .exe

else

PFX =
CC = gcc
EXESFX =

endif



ZIP = $(ZIPNAME)

all : hmckc

help :
	@echo "usage make [hmckc|xpcm|huc|husic]"


hmckc_clean :
	cd src/hmckc/src/ ; make clean

xpcm_clean :
	cd src/wav2pd4/ ; make clean

huc_clean :
	cd src/huc/ ; make clean

tests_clean :
	cd tests/ ; sh clean.sh

posix_clean :
	cd bin/ ; rm -f hmckc hmckc_j huc pceas xpcm

songs_clean :
	cd songs/ ; rm -f *.hes *.pce

hmckc :
	cd src/hmckc/src/ ; \
	make clean install -f Makefile.w32 ; \
	make clean -f Makefile.w32

xpcm :
	cd src/wav2pd4/ ; \
	make CC=$(CC) EXESFX=$(EXESFX) install ; \
	make EXESFX=$(EXESFX) clean

huc :
	cd src/huc/ ; \
	make CC=$(CC) EXESUFFIX=$(EXESFX); \
	make EXESUFFIX=$(EXESFX) clean

	cp src/huc/bin/huc$(EXESFX) bin/
	cp src/huc/bin/pceas$(EXESFX) bin/
	rm -f src/huc/bin/*

bin: hmckc xpcm huc

tests :
	cd tests/ ; sh build.sh

husic :
	cd src/husic/ ; sh compile.sh

full : bin husic

full_clean : huc_clean hmckc_clean xpcm_clean

clean: hmckc_clean songs_clean posix_clean


zip: distclean
	zip -r $(ZIP) . -x .DS\* .git\*

zipclean:
	rm -f $(ZIP)

distclean: zipclean clean
	rm -f src/huc/bin/*
	find . -name ".DS*" -delete
	find . -name "*.hes" -delete
	find . -name "*.pce" -delete
	find . -name "*.iso" -delete
