cmake_minimum_required(VERSION 3.20)
project(rpc VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")

enable_testing()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(INCLUDE_DIR
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include)



include_directories("~/CppSources/spdlog/include" "~/CppSources/tcp/include")
link_directories("~/CppSources/libs")

include_directories(${INCLUDE_DIR})
include_directories(BEFORE src)


file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# set(INCLUDE_FILES "include/*.h")
# set(SOURCE_FILES src/*.cpp)

add_library(${PROJECT_NAME} STATIC
        ${INCLUDE_FILES}
        ${SOURCE_FILES})

#add_library(${PROJECT_NAME}D SHARED
#        ${INCLUDE_FILES}
#        ${SOURCE_FILES})

add_subdirectory(tests)
add_subdirectory(third_party)