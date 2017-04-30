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
CMD = 

ifeq ($(OS),Windows_NT)
SCRSFX = .bat
EXESFX = .exe
CMD = cmd.exe /c
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

clean: hmckc_clean


hmckc :
	cd src/hmckc/src/ ; \
	make CC=$(CC) EXESFX=$(EXESFX) install ; \
	make EXESFX=$(EXESFX) clean

xpcm :
	cd src/wav2pd4/ ; \
	make CC=$(CC) EXESFX=$(EXESFX) install ; \
	make EXESFX=$(EXESFX) clean

huc :
	rm -rf src/huc/bin
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
	@echo \#define PRG_TITLE \"HuSIC $(DATE)\" > src/husic/title.h
	cd src/husic/ ; $(CMD) compile$(SCRSFX)

husic_dbg :
		cd src/husic/ ; compile_dbg$(SCRSFX)

full : bin husic


zip: strips distclean
	zip -r $(ZIP) . -x src\huc\* tests\output\* .DS\* .git\*

strips:
	strip bin/hmckc$(EXESFX)
	strip bin/hmckc_j$(EXESFX)
	strip bin/isolink$(EXESFX)
	strip bin/pceas$(EXESFX)
	strip bin/xpcm$(EXESFX)	

zipclean:
	rm -f $(ZIP)

distclean: zipclean clean
	rm -f src/huc/bin/*
	rm -f bin/huc$(EXESFX)
	