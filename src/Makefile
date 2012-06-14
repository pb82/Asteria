CXX       = g++
LIBS      = -L../google/bin/$(shell uname -m) -lv8 -lboost_thread -lprotobuf -ldl -lpthread
INCPATH   = -I./Include -I./Include/Shared -I../google/v8 -I../google -I../
CXXFLAGS  = -pipe -mtune=generic -O2 -pipe -fstack-protector --param=ssp-buffer-size=4 -D_FORTIFY_SOURCE=2 -Wall -W -D_REENTRANT -fPIC
MAJOR			= 1
MINOR			= 0
REVISION	= 0
LFLAGS		= -shared -Wl,-soname,libasteria.so.$(MAJOR)
BIN       = libasteria.so

SOURCES		= ScriptHandler.cpp Process.cpp PrettyPrinter.cpp NetworkEndpoint.cpp ModuleFinder.cpp ModuleCache.cpp Ipc.cpp EventQueue.cpp \
	ArrayEncoder.cpp \
	ArrayDecoder.cpp \
	Anchor.cpp \
	./Serialization/Event.pb.cc \
	./Shared/ByteBuffer.cpp

all:
	@make clean
	@$(CXX) $(CXXFLAGS) $(LFLAGS) $(INCPATH) $(LIBS) $(SOURCES) -o $(BIN).$(MAJOR).$(MINOR).$(REVISION)
	@make target

debug:
	@make clean
	@$(CXX) -DDEBUG $(CXXFLAGS) $(LFLAGS) $(INCPATH) $(LIBS) $(SOURCES) -o $(BIN).$(MAJOR).$(MINOR).$(REVISION)
	@make target

target:
	@ln -f -s $(BIN).$(MAJOR).$(MINOR).$(REVISION) $(BIN)
	@ln -f -s $(BIN).$(MAJOR).$(MINOR).$(REVISION) $(BIN).$(MAJOR)
	@ln -f -s $(BIN).$(MAJOR).$(MINOR).$(REVISION) $(BIN).$(MAJOR).$(MINOR)

install:
	@cp -d $(BIN)* /usr/local/lib/asteria

clean:
	@rm -f $(BIN)*
	@rm -f *.gch
	@rm -f *.o

