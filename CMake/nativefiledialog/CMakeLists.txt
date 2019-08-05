cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
set(PROJECT_NAME nativefiledialog)

IF (WIN32)
    set(OS_NAME "win")
ELSE()
    set(OS_NAME "linux")
ENDIF()

# Output Variables
set(OUTPUT_DEBUG ../../lib/${OS_NAME}/debug/)
set(OUTPUT_REL ../../lib/${OS_NAME}/release/)
# Folders files

set(CPP_DIR_1 ../../source/shared_code/NativeFileDialog/molflow_wrapper)
set(CPP_DIR_2 ../../source/shared_code/NativeFileDialog)
set(HEADER_DIR_1 ../../source/shared_code/NativeFileDialog/molflow_wrapper)
set(HEADER_DIR_2 ../../source/shared_code/NativeFileDialog)

################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} C CXX)

############## Artefacts Output ############################
# Defines outputs , depending BUILD TYPE                   #
############################################################

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

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

# Source Files
IF (WIN32)
    file(GLOB SRC_FILES
            ${CPP_DIR_1}/nfd_wrapper.cpp
            ${CPP_DIR_2}/nfd_common.c
            ${CPP_DIR_2}/nfd_win.cpp
            ${HEADER_DIR_1}/nfd_wrapper.h
            ${HEADER_DIR_2}/nfd.h
            ${HEADER_DIR_2}/nfd_common.h
            )
ELSE()
    file(GLOB SRC_FILES
            ${CPP_DIR_1}/nfd_wrapper.cpp
            ${CPP_DIR_2}/nfd_common.c
            ${CPP_DIR_2}/nfd_gtk.c
            ${HEADER_DIR_1}/nfd_wrapper.h
            ${HEADER_DIR_2}/nfd.h
            ${HEADER_DIR_2}/nfd_common.h
            )
ENDIF()

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        )

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})
target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES} )

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /MDd /Od /EHsc)
        #for .pdb debugging files
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FULL /OPT:REF /OPT:ICF")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /GL /Oi /Gy /EHsc)
    endif()
endif()