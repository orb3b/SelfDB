QT += core
QT -= gui

CONFIG += c++11

TARGET = Test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    Database.cpp \
    Calendar.cpp \
    Console.cpp \
    Server.cpp \
    Utilities.cpp

HEADERS += \
    Database.h \
    Calendar.h \
    Schema.h \
    Console.h \
    Server.h \
    Utilities.h
