

all: init client server twilio

CXX = cc
CFLAGS = -Wall
INCLUDE = -I/usr/local/include -I/usr/include -Iinclude
LDFLAGS = -L/usr/local/lib -I/usr/lib
LDLIBS = -lcurl

SOURCES = c_sms.c twilio.c

init: 
	mkdir -p build

client: 
	gcc -o build/client $(INCLUDE) client.c common.c

server: 
	gcc -o build/server $(INCLUDE) server.c common.c -lpthread

twilio: $(SOURCES)
	$(CXX) -o build/c_sms $(INCLUDE) $(LDFLAGS) $(CFLAGS) $(SOURCES) $(LDLIBS)
	

clean:
	 rm build/*
