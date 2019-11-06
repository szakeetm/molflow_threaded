cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

project(molflow)

option(USE_CLANG "build application with clang" OFF) # OFF is the default

message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")

string (REGEX MATCH "\\.el[1-9]" os_version_suffix ${CMAKE_SYSTEM})
message("-- os_version_suffix:      ${os_version_suffix}")

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE("Adding Debug flag...")
    ADD_DEFINITIONS(-DDEBUG)
endif()
message(STATUS "Build type used: " ${CMAKE_BUILD_TYPE})

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

# https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2019
if(MSVC)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    set(CMAKE_GENERATOR_PLATFORM x64)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /mwindows")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    # main vs WinMain
    # https://stackoverflow.com/questions/2752792/whats-the-equivalent-of-gccs-mwindows-option-in-cmake
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows /ENTRY:mainCRTStartup")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /EHsc")
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /MDd /Od /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /O2 /EHsc")
endif(MSVC)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif(NOT MSVC)

option(USE_PROFILING "disable optimisation for profiling the application" OFF) # OFF is the default
if(USE_PROFILING)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif(USE_PROFILING)




add_subdirectory(molflow_win)
add_subdirectory(compress)
add_subdirectory(pugixml)
add_subdirectory(clipper)
add_subdirectory(nativefiledialog)
add_subdirectory(sdl_savepng)
add_subdirectory(truncatedgaussian)

IF (WIN32)
    add_subdirectory(png_zlib_win_library)
ELSE()
    add_subdirectory(molflow_testsuit)
ENDIF(WIN32)

