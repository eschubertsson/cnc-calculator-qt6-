cmake_minimum_required(VERSION 3.16)

project(SponfortynningKalkulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(SponfortynningKalkulator
    main.cpp
    mainwindow.cpp
    mainwindow.h
    cnc_logic.cpp
    cnc_logic.h
)

target_link_libraries(SponfortynningKalkulator PRIVATE Qt6::Widgets)

if(WIN32)
    set_property(TARGET SponfortynningKalkulator PROPERTY WIN32_EXECUTABLE ON)
endif()

enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Test)
add_executable(ValidationTest tests/tst_validation.cpp cnc_logic.cpp cnc_logic.h)
target_link_libraries(ValidationTest PRIVATE Qt6::Test)
add_test(NAME ValidationTest COMMAND ValidationTest)
