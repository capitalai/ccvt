cmake_minimum_required(VERSION 3.19)
project(ccvt)
# set(CMAKE_VERBOSE_MAKEFILE ON)
if(WIN32)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CCVT_DATA_DIR=${CMAKE_INSTALL_PREFIX}/ccvt")
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CCVT_DATA_DIR=${CMAKE_INSTALL_PREFIX}/share/ccvt")
endif()
set(CMAKE_C_STANDARD 17)
include_directories(BEFORE "." "/usr/local/include/c++/v1")
# link_directories(BEFORE "/usr/local/lib")
# link_libraries(ccl)
file(GLOB PROJECT_SOURCE_FILES "src/*.c")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
if(WIN32)
install(DIRECTORY src/tc DESTINATION ${PROJECT_NAME})
install(DIRECTORY src/sc DESTINATION ${PROJECT_NAME})
else()
install(DIRECTORY src/tc DESTINATION share/${PROJECT_NAME})
install(DIRECTORY src/sc DESTINATION share/${PROJECT_NAME})
endif()
