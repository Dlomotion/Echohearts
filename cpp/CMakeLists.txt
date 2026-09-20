cmake_minimum_required(VERSION 3.20)
project(EchoheartsVerifiedSource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ECHOHEARTS_ENABLE_ASAN "Enable AddressSanitizer for supported compilers" OFF)

find_package(SQLite3 REQUIRED)

add_library(echohearts_meta INTERFACE)
target_include_directories(echohearts_meta INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(meta_systems_verify tests/MetaSystemsVerification.cpp)
target_link_libraries(meta_systems_verify PRIVATE echohearts_meta)

add_executable(object_pool_verify tests/ObjectPoolVerification.cpp)
target_link_libraries(object_pool_verify PRIVATE echohearts_meta)

add_executable(echo_verify tools/EchoheartsRepoVerify.cpp)
target_link_libraries(echo_verify PRIVATE SQLite::SQLite3)

set(ECHOHEARTS_VERIFICATION_TARGETS
    meta_systems_verify
    object_pool_verify
    echo_verify)

if(MSVC)
  foreach(target_name IN LISTS ECHOHEARTS_VERIFICATION_TARGETS)
    target_compile_options(${target_name} PRIVATE /W4 /WX)
  endforeach()
else()
  foreach(target_name IN LISTS ECHOHEARTS_VERIFICATION_TARGETS)
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endforeach()
endif()

if(ECHOHEARTS_ENABLE_ASAN)
  if(MSVC)
    foreach(target_name IN LISTS ECHOHEARTS_VERIFICATION_TARGETS)
      target_compile_options(${target_name} PRIVATE /fsanitize=address /Zi)
      target_link_options(${target_name} PRIVATE /INCREMENTAL:NO)
    endforeach()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    foreach(target_name IN LISTS ECHOHEARTS_VERIFICATION_TARGETS)
      target_compile_options(${target_name} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
      target_link_options(${target_name} PRIVATE -fsanitize=address)
    endforeach()
  else()
    message(FATAL_ERROR "ECHOHEARTS_ENABLE_ASAN is ON, but this compiler is not configured for AddressSanitizer")
  endif()
endif()

enable_testing()
add_test(NAME EchoheartsMetaSystemsVerification COMMAND meta_systems_verify)
add_test(NAME EchoheartsObjectPoolVerification COMMAND object_pool_verify)
