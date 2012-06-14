QT -= core gui

LIBS += -L../asteria-lib-bin -lasteria
TARGET = system
TEMPLATE = lib
CONFIG += dynamiclib

SOURCES += \
		../../Modules/system-module/system-module.cpp

