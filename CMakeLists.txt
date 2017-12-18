cmake_minimum_required(VERSION 2.8.11)

project(QDbf CXX)

FIND_PACKAGE(Qt5Core REQUIRED)

SET(CMAKE_AUTOMOC OFF)
SET(CMAKE_AUTORCC OFF)

ADD_DEFINITIONS(-DQDBF_LIBRARY)

option(BUILD_EXAMPLE "Build example" OFF)

if(BUILD_EXAMPLE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)
endif(BUILD_EXAMPLE)

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbf_compat.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbf_global.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbffield.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbfrecord.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbftable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbftablemodel.h
)

set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/qdbffield.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/qdbfrecord.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/qdbftable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/qdbftablemodel.cpp
)

set(MOC
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbftablemodel.h
)

QT5_WRAP_CPP(SOURCES ${MOC})

add_library(${PROJECT_NAME} SHARED
  ${HEADERS}
  ${SOURCES}
  ${MOC}
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
)

install(FILES ${HEADERS}
  DESTINATION include
)
