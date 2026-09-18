cmake_minimum_required(VERSION 3.26.5)

project(myDSSSD VERSION 1.0.0)

find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${ROOT_INCLUDE_DIRS})

file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(myDSSSD myDSSSD.cpp ${HEADERS} ${SOURCES})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
target_link_libraries(myDSSSD ${ROOT_LIBRARIES} ROOT::Minuit)
