QT += testlib core
QT -= gui
CONFIG += console c++14
CONFIG -= app_bundle
TARGET = tst_recordingsmodel
INCLUDEPATH += ../../src
DEFINES += SRCDIR=\\\"$$PWD/..\\\"
SOURCES += tst_recordingsmodel.cpp ../../src/htmlparser.cpp ../../src/recordingsmodel.cpp
HEADERS += ../../src/htmlparser.h ../../src/recordingsmodel.h