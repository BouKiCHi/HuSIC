#
# Makefile for HuSIC
#

# command arguments:
# make [hmckc|xpcm|huc|husic]
#

DATE = $(shell date +"%y%m%d")
ZIPNAME = husic_$(DATE).zip

PFX =
CC = gcc


ifeq ($(OS),Windows_NT)
SCRSFX = .bat
EXESFX = .exe
else
SCRSFX = .sh
EXESFX =
endif

ifeq ($(WIN32),1)

PFX = i586-mingw32-
CC = $(PFX)gcc
EXESFX = .exe

endif



ZIP = $(ZIPNAME)

all : hmckc

help :
	@echo "usage make [hmckc|xpcm|huc|husic|env]"
	@echo " env  = hmckc | xpcm "
	@echo " bin  = hmckc | xpcm | huc"


hmckc_clean :
	cd src/hmckc/src/ ; make clean

xpcm_clean :
	cd src/wav2pd4/ ; make clean

huc_clean :
	cd src/huc/ ; make clean
	cd src/huc/ ; make clean EXESUFFIX=.exe

tests_clean :
	cd tests/ ; sh clean.sh

posix_bin_clean :
	cd bin/ ; rm -f hmckc hmckc_j huc pceas xpcm

win32_huc_clean : huc_clean
	cd bin/ ; rm -f huc.exe

songs_clean :
	cd songs/ ; rm -f *.hes *.pce

full_clean : huc_clean hmckc_clean xpcm_clean

clean: hmckc_clean songs_clean


hmckc :
	cd src/hmckc/src/ ; \
	make CC=$(CC) EXESFX=$(EXESFX) install ; \
	make EXESFX=$(EXESFX) clean

xpcm :
	cd src/wav2pd4/ ; \
	make CC=$(CC) EXESFX=$(EXESFX) install ; \
	make EXESFX=$(EXESFX) clean

huc :
	rm -f src/huc/bin
	mkdir src/huc/bin

	cd src/huc/ ; \
	make CC=$(CC) EXESUFFIX=$(EXESFX); \
	make EXESUFFIX=$(EXESFX) clean

	cp src/huc/bin/huc$(EXESFX) bin/
	cp src/huc/bin/pceas$(EXESFX) bin/
	cp src/huc/bin/isolink$(EXESFX) bin/
	rm -f src/huc/bin/*

env: hmckc xpcm

bin: hmckc xpcm huc

tests :
	cd tests/ ; sh build.sh

husic :
	cd src/husic/ ; compile$(SCRSFX)

husic_dbg :
		cd src/husic/ ; compile_dbg$(SCRSFX)


full : bin husic


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
