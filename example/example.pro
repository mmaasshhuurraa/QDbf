TARGET = example

include(../application.pri)
include(../qdbf.pri)

INCLUDEPATH += \
    $$PWD \
    $$PWD/../src
DEPENDPATH += $$INCLUDEPATH

LIBS += -l$$qtLibraryName(QDbf)

HEADERS += mainwindow.h
SOURCES += main.cpp \
    mainwindow.cpp
