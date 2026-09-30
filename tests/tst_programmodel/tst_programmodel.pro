QT += testlib core gui
QT -= quick qml
CONFIG += console c++14
CONFIG -= app_bundle
TARGET = tst_programmodel

# libxml2 — use pkg-config when available, fall back to system paths
packagesExist(libxml-2.0) {
    PKGCONFIG += libxml-2.0
} else {
    INCLUDEPATH += /usr/include/libxml2
    LIBS += -lxml2
}
INCLUDEPATH += ../../src
DEFINES += SRCDIR=\\\"$$PWD/..\\\"
SOURCES += tst_programmodel.cpp ../../src/htmlparser.cpp ../../src/programmodel.cpp
HEADERS += ../../src/htmlparser.h ../../src/programmodel.h