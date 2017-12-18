TARGET = QDbfExample
TEMPLATE = app

include(../common.pri)
include(rpath.pri)

DESTDIR = $$BUILD_TREE/bin

LIBS *= -l$$qtLibraryName(QDbf)

HEADERS += \
    mainwindow.h

SOURCES += \
    main.cpp \
    mainwindow.cpp

target.path = /bin
INSTALLS += target
