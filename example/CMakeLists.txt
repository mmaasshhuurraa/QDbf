cmake_minimum_required(VERSION 2.8.11)

project(QDbfExample CXX)

if(QT_VERSION_MAJOR MATCHES 5)
  find_package(Qt5Gui REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  add_definitions(${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS})
else()
  find_package(Qt4 COMPONENTS QtGui REQUIRED)
endif()

set(HEADERS
  mainwindow.h
)

set(SOURCES
  mainwindow.cpp
  main.cpp
)

set(MOC_HEADERS
  mainwindow.h
)

if(QT_VERSION_MAJOR MATCHES 5)
  qt5_wrap_cpp(SOURCES ${MOC_HEADERS})
  qt5_generate_moc(mainwindow.h ${CMAKE_CURRENT_BINARY_DIR}/moc_mainwindow.cpp)
else()
  qt4_wrap_cpp(SOURCES ${MOC_HEADERS})
  qt4_generate_moc(mainwindow.h ${CMAKE_CURRENT_BINARY_DIR}/moc_mainwindow.cpp)
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
