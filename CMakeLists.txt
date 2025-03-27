cmake_minimum_required(VERSION 3.16)
project(ValArhivater VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(LibArchive REQUIRED)
find_package(ZLIB REQUIRED)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/archiver.cpp
)

set(HEADERS
    include/mainwindow.h
    include/archiver.h
)

set(UI_FILES
    resources/mainwindow.ui
)

set(RESOURCES
    resources/icons/icons.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    LibArchive::LibArchive
    ZLIB::ZLIB
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/resources
) 