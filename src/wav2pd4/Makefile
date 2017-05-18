
EXESFX =

TARGET = xpcm$(EXESFX)
OBJS = xpcm.o

INSTDIR = ../../bin


CFLAGS = -Wall -g
LDFLAGS = 

CC = gcc
LD = $(CC)
RM = rm -rf
CP = cp

%.o : %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET) : $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

clean : 
	$(RM) *.dSYM
	$(RM) $(OBJS)
	$(RM) $(TARGET)


install : $(TARGET)
	$(CP) $(TARGET) $(INSTDIR)

all : $(TARGET)
