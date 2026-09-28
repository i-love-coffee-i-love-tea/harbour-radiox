TARGET = harbour-radiox

CONFIG += sailfishapp
QT += quick qml network multimedia
PKGCONFIG += libxml-2.0

SOURCES += \
    src/main.cpp \
    src/htmlparser.cpp \
    src/networkfetcher.cpp \
    src/programmodel.cpp \
    src/recordingsmodel.cpp \
    src/sendetippsmodel.cpp \
    src/radioxcore.cpp

HEADERS += \
    src/htmlparser.h \
    src/networkfetcher.h \
    src/programmodel.h \
    src/recordingsmodel.h \
    src/sendetippsmodel.h \
    src/radioxcore.h

OTHER_FILES += \
    qml/harbour-radiox.qml \
    qml/pages/*.qml \
    qml/components/*.qml \
    qml/cover/*.qml \
    qml/js/*.js