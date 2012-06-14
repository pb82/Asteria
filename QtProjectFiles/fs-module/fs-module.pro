QT -= core \
    gui

LIBS += -L../asteria-lib-bin -lasteria

TARGET = fs
TEMPLATE = lib
CONFIG += dynamiclib

SOURCES += \
    ../../Modules/fs-module/fs-module.cpp

