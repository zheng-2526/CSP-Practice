cmake_minimum_required(VERSION 3.21)

project(QtCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(QtCalculator
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    calculatorengine.cpp
    calculatorengine.h
)

target_link_libraries(QtCalculator PRIVATE Qt6::Widgets)

set_target_properties(QtCalculator PROPERTIES
    WIN32_EXECUTABLE ON
)

enable_testing()

add_executable(CalculatorEngineTest
    tests/calculatorengine_test.cpp
    calculatorengine.cpp
    calculatorengine.h
)

target_include_directories(CalculatorEngineTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME CalculatorEngineTest COMMAND CalculatorEngineTest)

add_executable(CalculatorUiTest
    tests/calculatorui_test.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    calculatorengine.cpp calculatorengine.h
)
target_include_directories(CalculatorUiTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CalculatorUiTest PRIVATE Qt6::Widgets)
add_test(NAME CalculatorUiTest COMMAND CalculatorUiTest)
