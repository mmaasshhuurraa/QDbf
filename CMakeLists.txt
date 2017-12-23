cmake_minimum_required(VERSION 2.8.11)

project(QDbf CXX)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Qt
message("Searching for preferred Qt version...")

if(DEFINED QT_DESIRED_VERSION)
  if(QT_DESIRED_VERSION MATCHES 5)
    set(QT_VERSION_MAJOR 5)
    find_package(Qt5Core REQUIRED)
  else(QT_DESIRED_VERSION MATCHES 5)
    if(QT_DESIRED_VERSION MATCHES 4)
      find_package(Qt4 COMPONENTS QtCore REQUIRED)
      include(${QT_USE_FILE})
    else(QT_DESIRED_VERSION MATCHES 4)
      message(FATAL_ERROR "You must specify the 4th or the 5th version of Qt")
    endif(QT_DESIRED_VERSION MATCHES 4)
  endif(QT_DESIRED_VERSION MATCHES 5)
else(DEFINED QT_DESIRED_VERSION)
  find_package(Qt5Core QUIET)
  if(Qt5Core_FOUND)
    set(QT_VERSION_MAJOR 5)
  else(Qt5Core_FOUND)
    message("Qt 5 not found, searching for Qt4")
    find_package(Qt4 REQUIRED)
  endif(Qt5Core_FOUND)
endif(DEFINED QT_DESIRED_VERSION)

message("Qt version is used: ${QT_VERSION_MAJOR}")

if(QT_VERSION_MAJOR MATCHES 5)
  add_definitions(${Qt5Core_DEFINITIONS})
  include_directories(
    include
    src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Core_INCLUDE_DIRS}
  )
else()
  add_definitions(${QT_DEFINITIONS})
  include(${QT_USE_FILE})
  include_directories(
    include
    src
    ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

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

set(MOC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qdbftablemodel.h
)

if(QT_VERSION_MAJOR MATCHES 5)
  qt5_wrap_cpp(SOURCES ${MOC_HEADERS})
  qt5_generate_moc(include/qdbftablemodel.h
    ${CMAKE_CURRENT_BINARY_DIR}/moc_qdbftablemodel.cpp
  )
else()
  qt4_wrap_cpp(SOURCES ${MOC_HEADERS})
  qt4_generate_moc(include/qdbftablemodel.h
    ${CMAKE_CURRENT_BINARY_DIR}/moc_qdbftablemodel.cpp
  )
endif()

set_source_files_properties(src/qdbftablemodel.cpp PROPERTIES
  OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qdbftablemodel.cpp
)

add_definitions(-DQDBF_LIBRARY)

option(BUILD_EXAMPLE "Build example" OFF)

if(BUILD_EXAMPLE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)
endif(BUILD_EXAMPLE)

set(TARGET QDbf)

add_library(${TARGET} SHARED
  ${HEADERS}
  ${SOURCES}
  ${MOC_HEADERS}
)

if(QT_VERSION_MAJOR MATCHES 5)
  target_link_libraries(${TARGET}
    ${Qt5Core_LIBRARIES}
  )
else()
  target_link_libraries(${TARGET}
    ${QT_LIBRARIES}
  )
endif()

target_include_directories(${TARGET} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS ${TARGET}
  LIBRARY DESTINATION lib
)

install(FILES ${HEADERS}
  DESTINATION include
)
