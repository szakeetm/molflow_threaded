# see for tutorial: https://www.jetbrains.com/help/clion/unit-testing-tutorial.html#adding-framework
# adapt to extern loading gtest: https://cliutils.gitlab.io/modern-cmake/chapters/testing/googletest.html
cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME molflow_testsuit)

IF (WIN32)
    set(OS_NAME "win")
ELSE()
    IF(os_version_suffix STREQUAL ".el7")
        set(OS_NAME "linux_fedora")
    ELSE()
        set(OS_NAME "linux_debian")
    ENDIF()
ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MY_BUILD_TYPE "debug")
ELSE()
    set(MY_BUILD_TYPE "release")
ENDIF()

add_definitions(-DMOLFLOW_PATH="\\"/${OS_NAME}/${MY_BUILD_TYPE}/\\"")


# Output Variables
set(OUTPUT_DEBUG ../../bin/${OS_NAME}/debug/)
set(OUTPUT_REL ../../bin/${OS_NAME}/release/)

# Folders files
set(CPP_DIR_1 ../../source/gtest)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

add_subdirectory(../../lib_external/googletest build)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

############## Artefacts Output #################
# Defines outputs , depending Debug or Release. #
#################################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
endif()

# Messages
message("${PROJECT_NAME}: MAIN PROJECT: ${CMAKE_PROJECT_NAME}")
message("${PROJECT_NAME}: CURR PROJECT: ${CMAKE_CURRENT_SOURCE_DIR}")
message("${PROJECT_NAME}: CURR BIN DIR: ${CMAKE_CURRENT_BINARY_DIR}")

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

#[[if(MSVC)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    set(CMAKE_GENERATOR_PLATFORM x64)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /mwindows")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    # main vs WinMain
    # https://stackoverflow.com/questions/2752792/whats-the-equivalent-of-gccs-mwindows-option-in-cmake
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows /ENTRY:mainCRTStartup")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /EHsc")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /EHsc")
endif(MSVC)
if(NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
   endif()
endif(NOT MSVC)]]

if(MSVC)

endif(MSVC)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif(NOT MSVC)

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
        ${CPP_DIR_1}/*.cpp
        ${HEADER_DIR_1}/*.h
        ${HEADER_DIR_2}/*.h
        )



# set the path to the library folder
link_directories(${LINK_DIR_1}
        )

#find_library(PROTOBUF_LIBRARY bzip2 HINTS ${LINK_DIR_2})

#message(${EXECUTABLE_OUTPUT_PATH})

# Add executable to build.
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}  gtest gtest_main)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
#target_link_libraries(${PROJECT_NAME} Threads::Threads)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)