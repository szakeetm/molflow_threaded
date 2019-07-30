cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

# Project
set(PROJECT_NAME truncatedgaussian)

############## CMake Project ################
#        The main options of project        #
#############################################

IF (WIN32)
    set(OS_NAME "win")
ELSE()
    set(OS_NAME "linux")
ENDIF()

# Output Variables
set(OUTPUT_DEBUG ../../lib/${OS_NAME}/debug/)
set(OUTPUT_REL ../../lib/${OS_NAME}/release/)
# Folders files
set(CPP_DIR_1 ../../source/shared_code/TruncatedGaussian)
set(HEADER_DIR_1 ../../source/shared_code/TruncatedGaussian)

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

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
        ${CPP_DIR_1}/*.cpp
        ${HEADER_DIR_1}/*.hpp
        )

# Add library to build.
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})



target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        )

if(MSVC)
    target_include_directories(${PROJECT_NAME} PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/../../include/windows_only"
            )
endif(MSVC)
if(NOT MSVC)

    find_package(GSL REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GSL_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} ${GSL_LIBRARIES})

endif(NOT MSVC)



################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

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