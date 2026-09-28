QT += testlib core
QT -= gui
CONFIG += console c++14
CONFIG -= app_bundle
TARGET = tst_programmodel
INCLUDEPATH += ../../src
DEFINES += SRCDIR=\\\"$$PWD/..\\\"
SOURCES += tst_programmodel.cpp ../../src/htmlparser.cpp ../../src/programmodel.cpp
HEADERS += ../../src/htmlparser.h ../../src/programmodel.h