QT -= core \
    gui

LIBS += -L../asteria-lib-bin -lasteria

TARGET = binary
TEMPLATE = lib
CONFIG += dynamiclib

SOURCES += \
    ../../Modules/binary-module/binary-module.cpp

