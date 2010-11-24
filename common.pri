CONFIG += release
#CONFIG += debug
#CONFIG += debug_and_release build_all
CONFIG += qt warn-on

defineReplace(qtLibraryName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
          mac:RET = $$member(LIBRARY_NAME, 0)_debug
              else:win32:RET = $$member(LIBRARY_NAME, 0)d
      }
   }
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

CONFIG(debug, debug|release) {
    BIN_PATH = $$PWD/bin/debug
    OBJECTS_DIR = $$PWD/build/debug/$$TARGET/obj
    UI_DIR = $$PWD/build/debug/$$TARGET/uic
    MOC_DIR = $$PWD/build/debug/$$TARGET/moc
    RCC_DIR = $$PWD/build/debug/$$TARGET/rcc
} else {
    BIN_PATH = $$PWD/bin/release
    OBJECTS_DIR = $$PWD/build/release/$$TARGET/obj
    UI_DIR = $$PWD/build/release/$$TARGET/uic
    MOC_DIR = $$PWD/build/release/$$TARGET/moc
    RCC_DIR = $$PWD/build/release/$$TARGET/rcc
}

LIBRARY_PATH = $$PWD/lib
LIBS += -L$$LIBRARY_PATH
