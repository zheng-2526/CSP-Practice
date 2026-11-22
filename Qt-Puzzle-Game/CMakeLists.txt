cmake_minimum_required(VERSION 3.21)

project(QtPuzzleGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(QtPuzzleGame
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    puzzlewidget.cpp
    puzzlewidget.h
    puzzlemodel.cpp
    puzzlemodel.h
    resources.qrc
)

target_link_libraries(QtPuzzleGame PRIVATE Qt6::Widgets)
target_include_directories(QtPuzzleGame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(QtPuzzleGame PROPERTIES
    WIN32_EXECUTABLE ON
)

enable_testing()

add_executable(PuzzleModelTest
    tests/puzzlemodel_test.cpp
    puzzlemodel.cpp
    puzzlemodel.h
)

target_include_directories(PuzzleModelTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME PuzzleModelTest COMMAND PuzzleModelTest)

add_executable(PuzzleUiTest
    tests/puzzleui_test.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    puzzlewidget.cpp puzzlewidget.h
    puzzlemodel.cpp puzzlemodel.h resources.qrc
)
target_include_directories(PuzzleUiTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(PuzzleUiTest PRIVATE Qt6::Widgets)
add_test(NAME PuzzleUiTest COMMAND PuzzleUiTest)
