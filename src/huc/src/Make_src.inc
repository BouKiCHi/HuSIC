# Include file for Makefile for HuC
# Written for unified version, October 16, 2001 by Dave Shadoff 
#
# This file set initial defines, both general and platform-specific
#


#
# general stuff
#
CDEFS =
CFLAGS =
# CFLAGS += -Werror -Wall 
CFLAGS += -g -m32
LDFLAGS = -m32 -g

#
# platform-specific stuff
#
ifeq "$(origin DJGPP)" "undefined"

# BASEDIR uses 'd' for path separator; we fix that here
#
ROOTDIR=$(subst d ,/,$(BASEDIR))
BINDIR=$(ROOTDIR)/bin
MAKETARG=$(ROOTDIR)/src/Maketarg.inc
EXESUFFIX = 
CP = cp
RM = rm -f

else

# BASEDIR uses 'd' for path separator; we fix that here
#
ROOTDIR=$(subst d ,\,$(BASEDIR))
BINDIR=$(ROOTDIR)\bin
MAKETARG=$(ROOTDIR)\src\Maketarg.inc
EXESUFFIX = .exe
CP = copy
RM = del

endif

