cmake_minimum_required(VERSION 2.8.11)

project(QDbfExample CXX)

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
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Widgets REQUIRED)
  else(Qt5Core_FOUND)
    message("Qt 5 not found, searching for Qt4")
    find_package(Qt4 REQUIRED)
  endif(Qt5Core_FOUND)
endif(DEFINED QT_DESIRED_VERSION)

message("Qt version is used: ${QT_VERSION_MAJOR}")

if(QT_VERSION_MAJOR MATCHES 5)
    add_definitions(
      ${Qt5Core_DEFINITIONS}
      ${Qt5Gui_DEFINITIONS}
      ${Qt5Widgets_DEFINITIONS}
    )
  include_directories(
    include
    src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
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
  ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
)

set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set(MOC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
)

if(QT_VERSION_MAJOR MATCHES 5)
  qt5_wrap_cpp(SOURCES ${MOC_HEADERS})
  qt5_generate_moc(mainwindow.h
    ${CMAKE_CURRENT_BINARY_DIR}/moc_mainwindow.cpp
  )
else()
  qt4_wrap_cpp(SOURCES ${MOC_HEADERS})
  qt4_generate_moc(mainwindow.h
    ${CMAKE_CURRENT_BINARY_DIR}/moc_mainwindow.cpp
  )
ENDIF()

set_source_files_properties(mainwindow.cpp PROPERTIES
  OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_mainwindow.cpp
)

set(TARGET QDbfExample)

add_executable(${TARGET}
  ${HEADERS}
  ${SOURCES}
  ${MOC}
)

if(QT_VERSION_MAJOR MATCHES 5)
  target_link_libraries(${TARGET}
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    QDbf
  )
else()
  target_link_libraries(${TARGET}
    ${QT_LIBRARIES}
    QDbf
  )
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
