cmake_minimum_required(VERSION 3.0.0)
project(stockmarket VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

add_executable(stockmarket main.cpp)
add_executable(client client.cpp)

target_compile_options(stockmarket PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic -Werror)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(fmt)
target_link_libraries(stockmarket 
  PRIVATE 
    fmt::fmt 
)
