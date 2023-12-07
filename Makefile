CC=g++
CFLAGS=-I. #-lsctp
#CFLAGS=

OBJECTS = warcaby_client warcaby_serwer

all: $(OBJECTS)

$(OBJECTS):%:%.cpp
	@echo Compiling $<  to  $@
	$(CC) -o $@ $< $(CFLAGS)

	
clean:
	rm  $(OBJECTS) 
