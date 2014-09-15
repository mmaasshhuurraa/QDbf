include(common.pri)

win32 {
    DLLDESTDIR = $$APP_PATH
}

DESTDIR = $$LIBRARY_PATH

include(rpath.pri)

TARGET = $$qtLibraryName($$TARGET)

TEMPLATE = lib
CONFIG += shared dll

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

!macx {
    win32 {
        dlltarget.path = /bin
        INSTALLS += dlltarget
    } else {
        target.path = /$$LIBRARY_BASENAME
        INSTALLS += target
    }
}
