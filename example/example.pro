include(../common.pri)

TEMPLATE = app
TARGET = QDbfExample
DESTDIR = $$BUILD_TREE/bin

include(../rpath.pri)

LIBS *= -l$$qtLibraryName(QDbf)

HEADERS += \
    mainwindow.h
SOURCES += \
    main.cpp \
    mainwindow.cpp

target.path = /bin
INSTALLS += target
