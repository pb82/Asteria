CXX				= g++
LIBS      = -L../google/bin/$(shell uname -m) -L../src -lv8 -lprotobuf -lasteria -lpthread
INCPATH   = -I./Include -I./Include/Shared -I../google/v8 -I../google -I../
CXXFLAGS	= -pipe -mtune=generic -O2 -pipe -fstack-protector --param=ssp-buffer-size=4 -D_FORTIFY_SOURCE=2 -Wall -W -D_REENTRANT
BIN				=	asteria-run

SOURCES		= RequestMapper.cpp MainLoop.cpp main.cpp

all:
	@$(CXX) $(CXXFLAGS) $(INCPATH) $(LIBS) $(SOURCES) -o $(BIN)

debug:
	@$(CXX) -DDEBUG $(CXXFLAGS) $(INCPATH) $(LIBS) $(SOURCES) -o $(BIN)

install:
	@cp -p $(BIN) /usr/local/bin

clean:
	@rm -f $(BIN)
	@rm -f *.gch
	@rm -f *.o
