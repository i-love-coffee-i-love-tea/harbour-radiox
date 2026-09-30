QT += testlib core gui
QT -= quick qml
CONFIG += console c++14
CONFIG -= app_bundle
TARGET = tst_recordingsmodel

# libxml2 — use pkg-config when available, fall back to system paths
packagesExist(libxml-2.0) {
    PKGCONFIG += libxml-2.0
} else {
    INCLUDEPATH += /usr/include/libxml2
    LIBS += -lxml2
}
INCLUDEPATH += ../../src
DEFINES += SRCDIR=\\\"$$PWD/..\\\"
SOURCES += tst_recordingsmodel.cpp ../../src/htmlparser.cpp ../../src/recordingsmodel.cpp
HEADERS += ../../src/htmlparser.h ../../src/recordingsmodel.h