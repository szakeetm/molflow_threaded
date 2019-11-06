cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

# Project
set(PROJECT_NAME sdl_savepng)
IF (WIN32)
    set(OS_NAME "win")
ELSE()
    IF(os_version_suffix STREQUAL ".el7")
        set(OS_NAME "linux_fedora")
    ELSE()
        set(OS_NAME "linux_debian")
    ENDIF()
ENDIF()

# Output Variables
set(OUTPUT_DEBUG ../../lib/${OS_NAME}/debug/)
set(OUTPUT_REL ../../lib/${OS_NAME}/release/)

# Folders files
set(CPP_DIR_1 ../../source/shared_code/SDL_SavePNG)
set(HEADER_DIR_1 ../../source/shared_code/SDL_SavePNG)
set(HEADER_DIR_2 ../../include/windows_only)
set(HEADER_DIR_3 ../../include/windows_only/png)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

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
# Messages
message("${PROJECT_NAME}: CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message("${PROJECT_NAME}: CMAKE_ARCHIVE_OUTPUT_DIRECTORY: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message("${PROJECT_NAME}: CMAKE_EXECUTABLE_OUTPUT_DIRECTORY: ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}")

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
        ${CPP_DIR_1}/*.cpp
        ${HEADER_DIR_1}/*.h
        )

# Add library to build.
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})


# Source Files
IF (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${HEADER_DIR_1}
            ${HEADER_DIR_2}
            ${HEADER_DIR_3}
            )

ELSE()
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${HEADER_DIR_1}
            )
ENDIF()

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /MDd /Od /EHsc)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FULL /OPT:REF /OPT:ICF")
        #set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "-DEBUG:FULL")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /GL /Oi /Gy /EHsc)
    endif()
endif()