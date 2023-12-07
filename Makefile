CC=gcc
CFLAGS=-I. #-lsctp
#CFLAGS=

OBJECTS = warcaby_client warcaby_serwer

all: $(OBJECTS)

$(OBJECTS):%:%.c
	@echo Compiling $<  to  $@
	$(CC) -o $@ $< $(CFLAGS)

	
clean:
	rm  $(OBJECTS) 
