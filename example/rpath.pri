unix {
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,\$\$ORIGIN/../$$LIBRARY_BASENAME\'
    QMAKE_RPATHDIR =
}
