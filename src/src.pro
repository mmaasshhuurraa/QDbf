TARGET = QDbf

QT -= gui
TEMPLATE = lib
CONFIG += shared dll

include(../common.pri)

DEFINES += QDBF_LIBRARY
TARGET = $$qtLibraryName($$TARGET)
DESTDIR = $$LIBRARY_PATH
win32 {
    DLLDESTDIR = $$BUILD_TREE/bin
}
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

HEADERS += \
    $$SOURCE_TREE/include/qdbffield.h \
    $$SOURCE_TREE/include/qdbfrecord.h \
    $$SOURCE_TREE/include/qdbftable.h \
    $$SOURCE_TREE/include/qdbftablemodel.h \
    $$SOURCE_TREE/include/qdbf_compat.h \
    $$SOURCE_TREE/include/qdbf_global.h

SOURCES += \
    $$SOURCE_TREE/src/qdbffield.cpp \
    $$SOURCE_TREE/src/qdbfrecord.cpp \
    $$SOURCE_TREE/src/qdbftable.cpp \
    $$SOURCE_TREE/src/qdbftablemodel.cpp

!macx {
    win32 {
        dlltarget.path = /bin
        INSTALLS += dlltarget
    } else {
        target.path = /$$LIBRARY_BASENAME
        INSTALLS += target
    }
}
