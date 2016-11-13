cmake_minimum_required(VERSION 3.1)

project(universal_machine)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 14)

find_package(Readline REQUIRED)
find_package(libzmq REQUIRED)


include_directories(${READLINE_INCLUDE_DIR} ${LIBZMQ_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/cppzmq")

add_executable(um main_vm.cpp)
target_link_libraries(um ${LIBZMQ_LIBRARIES})

add_executable(dum main_dum.cpp)
target_link_libraries(dum ${LIBZMQ_LIBRARIES})

add_executable(udb main_udb.cpp)
target_link_libraries(udb ${READLINE_LIBRARIES} ${LIBZMQ_LIBRARIES})

execute_process(COMMAND codesign -f -s synacor udb --deep)
