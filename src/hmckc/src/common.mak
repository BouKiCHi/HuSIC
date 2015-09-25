
ifdef DEBUG 

CFLAGS += -g
CFLAGS += -Wall# -mno-cygwin

else

CFLAGS += -O2

endif

CDEFS = 
LDFLAGS= # -mno-cygwin

OBJS = datamake.o file.o mckc.o strings.o



.PHONY: all clean
PRGNAME = hmckc

TARGET_E = $(EXEDIR)$(PRGNAME)$(EXESFX)
TARGET = $(EXEDIR)$(PRGNAME)_j$(EXESFX)

all: $(TARGET_E) $(TARGET)

$(TARGET_E): $(OBJS) version_e.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) version_e.o
#	strip $@

$(TARGET): $(OBJS) version.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) version.o
#	strip $@

version_e.o: version.c
	$(CC) $(CFLAGS) $(CDEFS) -DENGLISH -o $@ -c version.c

.c.o:
	$(CC) $(CFLAGS) $(CDEFS) -c $<

mckc.o: mckc.h
datamake.o: mckc.h

clean:
	$(RM) $(OBJS) version.o version_e.o $(TARGET_E) $(TARGET)
	
install: all
	$(CP) $(TARGET_E) $(INST_DIR)
	$(CP) $(TARGET) $(INST_DIR)
