cmake_minimum_required(VERSION 2.8.11)

project(QDbfExample CXX)

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
FIND_PACKAGE(Qt5Widgets REQUIRED)

SET(CMAKE_AUTOMOC OFF)
SET(CMAKE_AUTORCC OFF)

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
)

set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set(MOC
  ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
)

QT5_WRAP_CPP(SOURCES ${MOC})

add_executable(${PROJECT_NAME}
  ${HEADERS}
  ${SOURCES}
  ${MOC}
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  QDbf
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
