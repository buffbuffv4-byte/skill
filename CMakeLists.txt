cmake_minimum_required(VERSION 3.21)
project(SkillOpti VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)
qt_add_executable(SkillOpti WIN32
    main.cpp MainWindow.cpp MainWindow.h
    SystemMonitor.cpp SystemMonitor.h
    SystemScanner.cpp SystemScanner.h
    Cleaner.cpp Cleaner.h
    resources.qrc
)
target_link_libraries(SkillOpti PRIVATE Qt6::Widgets)
target_compile_definitions(SkillOpti PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
if(MSVC)
  target_compile_options(SkillOpti PRIVATE /W4 /permissive-)
endif()
