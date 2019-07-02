cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME molflow)
# Output Variables
set(OUTPUT_DEBUG ../../bin/win/debug/)
set(OUTPUT_REL ../../bin/win/release/)
# Folders files
set(CPP_DIR_1 ../../source/molflow_code)
set(CPP_DIR_2 ../../source/shared_code)
set(CPP_DIR_3 ../../source/shared_code/GLApp)
set(CPP_DIR_4 ../../source/shared_code/GLApp/GLChart)
set(CPP_DIR_5 ../../source/shared_code/windows_only)
set(HEADER_DIR_1 ../../source/molflow_code)
set(HEADER_DIR_2 ../../source/shared_code)
set(HEADER_DIR_3 ../../source/shared_code/GLApp)
set(HEADER_DIR_4 ../../source/shared_code/GLApp/GLChart)
set(HEADER_DIR_5 ../../include/)
set(HEADER_DIR_6 ../../include/windows_only)
set(HEADER_DIR_7 ../../include/windows_only/png)
set(LINK_DIR_1 ../../lib/win)
set(LINK_DIR_2 ../../lib/win/release)
set(DLL_DIR ../../lib/win/dll)
set(IMAGE_DIR ../../images)


############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

# Definition of Macros
add_definitions(
   -DCURL_STATICLIB 
   -D_MBCS 
   -DMOLFLOW 
   -D_CRT_SECURE_NO_WARNINGS 
   -D_CRT_NONSTDC_NO_DEPRECATE 
)

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

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
    ${CPP_DIR_1}/*.cpp
    ${CPP_DIR_2}/*.cpp
    ${CPP_DIR_3}/*.cpp
    ${CPP_DIR_4}/*.cpp
    ${CPP_DIR_5}/*.cpp
    ${HEADER_DIR_1}/*.h
    ${HEADER_DIR_2}/*.h
    ${HEADER_DIR_3}/*.h
    ${HEADER_DIR_4}/*.h
)



# set the path to the library folder
link_directories(${LINK_DIR_1}
        ${LINK_DIR_2}
        )

#find_library(PROTOBUF_LIBRARY bzip2 HINTS ${LINK_DIR_2})


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}) #to build executable in main folder
#message(${EXECUTABLE_OUTPUT_PATH})

# Add executable to build.
add_executable(${PROJECT_NAME} WIN32
   ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        ${HEADER_DIR_3}
        ${HEADER_DIR_4}
        ${HEADER_DIR_5}
        ${HEADER_DIR_6}
        ${HEADER_DIR_7}
        )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# Multi-processor compilation
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
            "$<$<CONFIG:Debug>:/MP>"
            "$<$<CONFIG:Release>:/MP>"
            )
endif ()

if(MSVC)
    # Add Whole Program Optimization and Link Time Code Generation
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/GL")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/LTCG")

    target_link_libraries(${PROJECT_NAME} bzip2.lib clipper.lib libcurl_a_x64.lib SDL2.lib SDL2main.lib libgsl.lib libgslcblas.lib lzma.lib png_zlib_win_library.lib pugixml.lib ZipLib.lib zlib.lib nativefiledialog.lib sdl_savepng.lib truncatedgaussian.lib opengl32.lib user32.lib shell32.lib ole32.lib )
    target_link_libraries(${PROJECT_NAME} pugixml clipper nativefiledialog sdl_savepng truncatedgaussian png_zlib_win_library)
endif(MSVC)

file(GLOB DLL_FILES
        ${DLL_DIR}/*.dll
)

file(REMOVE_RECURSE ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY})

file(COPY ${DLL_FILES} ${IMAGE_DIR}
        DESTINATION ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY})