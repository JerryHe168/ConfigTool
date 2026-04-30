cmake_minimum_required(VERSION 3.10)
project(ConfigTool1Tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(test_basic test_basic.cpp)
add_executable(test_types test_types.cpp)
add_executable(test_validation test_validation.cpp)
add_executable(test_include test_include.cpp)

target_link_libraries(test_basic PRIVATE IniConfigLib)
target_link_libraries(test_types PRIVATE IniConfigLib)
target_link_libraries(test_validation PRIVATE IniConfigLib)
target_link_libraries(test_include PRIVATE IniConfigLib)

target_compile_features(test_basic PRIVATE cxx_std_17)
target_compile_features(test_types PRIVATE cxx_std_17)
target_compile_features(test_validation PRIVATE cxx_std_17)
target_compile_features(test_include PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(test_basic PRIVATE /W4 /WX-)
    target_compile_options(test_types PRIVATE /W4 /WX-)
    target_compile_options(test_validation PRIVATE /W4 /WX-)
    target_compile_options(test_include PRIVATE /W4 /WX-)
else()
    target_compile_options(test_basic PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(test_types PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(test_validation PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(test_include PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME TestBasic COMMAND test_basic)
add_test(NAME TestTypes COMMAND test_types)
add_test(NAME TestValidation COMMAND test_validation)
add_test(NAME TestInclude COMMAND test_include)
