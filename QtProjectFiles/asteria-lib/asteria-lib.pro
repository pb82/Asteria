PLATFORM_LIBS = $$system(uname -m)

QT -= core gui
LIBS += -L ../../google/bin/$$PLATFORM_LIBS -lv8 -lboost_thread -lprotobuf -ldl
TARGET = asteria
TEMPLATE = lib
CONFIG += dynamiclib
DEFINES += DEBUG

SOURCES += \
    ../../src/ScriptHandler.cpp \
    ../../src/Process.cpp \
    ../../src/PrettyPrinter.cpp \
    ../../src/NetworkEndpoint.cpp \
    ../../src/ModuleFinder.cpp \
    ../../src/ModuleCache.cpp \
    ../../src/Ipc.cpp \
    ../../src/EventQueue.cpp \
    ../../src/ArrayEncoder.cpp \
    ../../src/ArrayDecoder.cpp \
    ../../src/Anchor.cpp \
    ../../src/Serialization/Event.pb.cc \
    ../../src/Shared/ByteBuffer.cpp

HEADERS += \
    ../../src/Include/ScriptHandler.h \
    ../../src/Include/Process.h \
    ../../src/Include/PrettyPrinter.h \
    ../../src/Include/Plugin.h \
    ../../src/Include/NetworkEndpoint.h \
    ../../src/Include/ModuleInterface.h \
    ../../src/Include/ModuleFinder.h \
    ../../src/Include/ModuleCache.h \
    ../../src/Include/Macros.h \
    ../../src/Include/Ipc.h \
    ../../src/Include/EventQueue.h \
    ../../src/Include/ArrayEncoder.h \
    ../../src/Include/ArrayDecoder.h \
    ../../src/Include/Anchor.h \
    ../../src/Include/Shared/ByteBuffer.h \
    ../../src/Serialization/Event.pb.h \
    ../../src/Include/ConsoleLogger.h \
    ../../src/Include/JsUtils.h







