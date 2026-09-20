cmake_minimum_required(VERSION 3.20)
project(EchoheartsVerifiedSource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)

add_library(echohearts_meta INTERFACE)
target_include_directories(echohearts_meta INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(meta_systems_verify tests/MetaSystemsVerification.cpp)
target_link_libraries(meta_systems_verify PRIVATE echohearts_meta)

add_executable(echo_verify tools/EchoheartsRepoVerify.cpp)
target_link_libraries(echo_verify PRIVATE SQLite::SQLite3)

if(MSVC)
  target_compile_options(meta_systems_verify PRIVATE /W4 /WX)
  target_compile_options(echo_verify PRIVATE /W4 /WX)
else()
  target_compile_options(meta_systems_verify PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(echo_verify PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME EchoheartsMetaSystemsVerification COMMAND meta_systems_verify)
