QT += testlib core
QT -= gui
CONFIG += console c++14
CONFIG -= app_bundle
TARGET = tst_htmlparser
INCLUDEPATH += ../../src
DEFINES += SRCDIR=\\\"$$PWD/..\\\"
SOURCES += tst_htmlparser.cpp ../../src/htmlparser.cpp
HEADERS += ../../src/htmlparser.h