# mckc makefile for MinGW and POSIX

# WIN32 = 1
# DEBUG = 1


CC = gcc
CFLAGS =
EXEDIR=
INST_DIR = ../../../bin
CP = cp
RM = rm -f

ifeq ($(OS),Windows_NT)
EXESFX = .exe
CFLAGS += --input-charset=utf-8 --exec-charset=cp932
else
#posix
CFLAGS += -DUNIX
EXESFX =
endif


include common.mak
