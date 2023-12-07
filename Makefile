CC=g++
CFLAGS=-I. -std=c++20#-lsctp
#CFLAGS=

OBJECTS = warcaby_client warcaby_serwer

all: $(OBJECTS)

$(OBJECTS):%:%.cpp
	@echo Compiling $<  to  $@
	$(CC) -o $@ $< $(CFLAGS)

	
clean:
	rm  $(OBJECTS) 
