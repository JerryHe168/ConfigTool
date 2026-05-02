cmake_minimum_required(VERSION 3.10)
project(ConfigTool1 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build test programs" ON)

include_directories(include)

set(LIB_SOURCES
    src/IniParser.cpp
    src/IniConfig.cpp
    src/IniValidator.cpp
    src/JsonParser.cpp
)

set(LIB_HEADERS
    include/IniParser.h
    include/IniConfig.h
    include/IniValidator.h
    include/IniExceptions.h
    include/JsonParser.h
)

add_library(IniConfigLib STATIC ${LIB_SOURCES} ${LIB_HEADERS})

set(MAIN_SOURCES
    src/main.cpp
)

add_executable(ConfigTool1 ${MAIN_SOURCES})
target_link_libraries(ConfigTool1 PRIVATE IniConfigLib)

target_compile_features(ConfigTool1 PRIVATE cxx_std_17)
target_compile_features(IniConfigLib PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ConfigTool1 PRIVATE /W4 /WX-)
    target_compile_options(IniConfigLib PRIVATE /W4 /WX-)
else()
    target_compile_options(ConfigTool1 PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(IniConfigLib PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
