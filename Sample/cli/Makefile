CC      =  gcc
CFLAGS  = -g  
LDFLAGS = -lPosixmsgq -lpthread -lrt 
TARGET  =  writer.BIN 
MODULE_LIST  =  writer.BIN reader.BIN
SOURCE_LIST = cliwriter.c clireader.c
all: 
	for m in $(SOURCE_LIST) ; \
	do  \
	$(CC) $$m $(CFLAGS) $(LDFLAGS)  -o $$m.BIN; \
	cp $$m.BIN ./../../bin ; \
	done 

install:
clean:
	rm -f *~
	rm -f *.o
	rm -f *.BIN
