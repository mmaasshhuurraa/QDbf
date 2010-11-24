include(common.pri)

TEMPLATE = lib
CONFIG += dll

DLLDESTDIR = $$BIN_PATH
DESTDIR = $$LIBRARY_PATH

isEmpty(TARGET) {
    error("library.pri: You must provide a TARGET")
}

TARGET = $$qtLibraryTarget($$TARGET)
