PLATFORM_LIBS = $$system(uname -m)

QT -= core gui
LIBS += -L../asteria-lib-bin
LIBS += -L../../google/bin/$$PLATFORM_LIBS -lasteria -lprotobuf
TARGET = asteria-run

HEADERS += \
    ../../Run/RequestMapper.h \
    ../../Run/MainLoop.h \
    ../../Run/Include/RequestMapper.h \
    ../../Run/Include/MainLoop.h

SOURCES += \
    ../../Run/RequestMapper.cpp \
    ../../Run/main.cpp \
    ../../Run/MainLoop.cpp












