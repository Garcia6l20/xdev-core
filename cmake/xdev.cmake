cmake_minimum_required(VERSION 3.12)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(XDEV_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})

include(GenerateExportHeader)
include(FetchContent)
include(CTest)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/find)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CTEST_OUTPUT_ON_FAILURE ON)

option(XDEV_UNIT_TESTING ON "Enable unit testing")

if(NOT TARGET RERUN_CMAKE)
    add_custom_target(RERUN_CMAKE COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR})
    set_target_properties(RERUN_CMAKE PROPERTIES FOLDER CMakePredefinedTargets)
endif()

find_program(VALGRIND_EXE valgrind)
if (VALGRIND_EXE)
    set(CTEST_MEMORYCHECK_COMMAND "${VALGRIND_EXE}")
endif()

string(STRIP ${CMAKE_CXX_COMPILER_VERSION} XDEV_CXX_COMPILER_VERSION)
string(REGEX REPLACE "^([0-9]+).*$" "\\1"
       XDEV_CXX_COMPILER_VERSION_MAJOR ${XDEV_CXX_COMPILER_VERSION})
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2"
       XDEV_CXX_COMPILER_VERSION_MINOR ${XDEV_CXX_COMPILER_VERSION})
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3"
       XDEV_CXX_COMPILER_VERSION_PATCH ${XDEV_CXX_COMPILER_VERSION})

#
# Setup moccing for the given @target
#
# @param target The target to moc
#
function(xdev_moc_target target)
    set(path "${ARGV1}")
    if (path)
        set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/${path})
    else()
        set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    get_target_property(header_files ${target} SOURCES)
    list(FILTER header_files INCLUDE REGEX ".*\.h(pp)?$")
    set(stamp ${CMAKE_CURRENT_BINARY_DIR}/${target}-moc-stamp)
    set(generated_files "")
    foreach(file ${header_files})
        get_filename_component(filename ${file} NAME_WE)
        list(APPEND generated_files ${CMAKE_CURRENT_BINARY_DIR}/${filename}.xdev.hpp)
        list(APPEND generated_files ${CMAKE_CURRENT_BINARY_DIR}/${filename}.xdev.cpp)
    endforeach()
    list(APPEND generated_files ${CMAKE_CURRENT_BINARY_DIR}/${target}-pools.xdev.hpp)
    list(APPEND generated_files ${CMAKE_CURRENT_BINARY_DIR}/${target}-pools.xdev.cpp)
    set(full_path_header_files ${header_files})
    list(TRANSFORM full_path_header_files PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)
    message("Moc[${target}] files: ${full_path_header_files}")
    add_custom_command(
        OUTPUT ${generated_files}
        DEPENDS "xdev-moc;${header_files}"
        COMMAND xdev-moc ARGS ${target} ${full_path_header_files}
    )
    source_group(xdev/moc FILES ${generated_files})
    target_sources(${target} PRIVATE ${generated_files})
    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
endfunction()

#
# Setup managed resource directory
#
# @param target The target requiring the managed directory
# @param directory The directory to manage
#
function(xdev_resources target directory)
    file(GLOB_RECURSE resource_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${directory}/*)
    set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/${directory})
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir})
    foreach(file ${resource_files})
        string(REPLACE ${directory}/ "" filename ${file})
        set(output_file ${output_dir}/${filename}.xrc)
        list(APPEND generated_files ${output_file})
    endforeach()
    list(APPEND generated_files ${output_dir}/${target}-resources.hpp)
    list(APPEND generated_files ${output_dir}/${target}-resources.cpp)
    add_custom_command(
        OUTPUT ${generated_files}
        DEPENDS "xdev-rc;${resource_files}"
        COMMAND xdev-rc ARGS ${target} ${CMAKE_CURRENT_SOURCE_DIR}/${directory}
        COMMENT "Generating resources for ${target}"
        WORKING_DIRECTORY ${output_dir}
    )
    message(STATUS "${target}: ${generated_files}")
    source_group(xdev/resources FILES ${resource_files})
    source_group(xdev/resources/generated FILES ${generated_files})
    target_sources(${target} PRIVATE ${generated_files} ${resource_files})
    target_include_directories(${target} PUBLIC ${output_dir})
endfunction()

find_package(Threads REQUIRED)

macro(_link_if_is_library target library)
if (TARGET ${library})
    get_target_property(library_type ${library} TYPE)
    if (library_type MATCHES ".*_LIBRARY")
        # message(STATUS "Autolinking: ${library} -> ${target}")
        target_link_libraries(${target} PRIVATE ${library})
    endif()
endif()
endmacro()

macro(xdev_add_test)

    if(NOT XDEV_UNIT_TESTING)
        return()
    endif()

    set(options NO_GTEST XMOC)
    set(oneValueArgs NAME WORKING_DIRECTORY)
    set(multiValueArgs FILES LINK_LIBRARIES XRES_DIRS)
    cmake_parse_arguments(TEST
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    set(TEST_TARGET ${PROJECT_NAME}-${TEST_NAME})
    # add cache variable to disable this test
    set(_no_test_var_name "${PROJECT_NAME}_NO_${TEST_NAME}")
    string(TOUPPER ${_no_test_var_name} _no_test_var_name)
    string(REPLACE "-" "_" _no_test_var_name ${_no_test_var_name})
    set(${_no_test_var_name} OFF CACHE BOOL "Disable ${PROJECT_NAME}-${TEST_NAME} while running tests")
#    message(STATUS "TEST_TARGET: ${TEST_TARGET}")
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/${TEST_NAME}.cpp)
        list(APPEND TEST_FILES ${TEST_NAME}.cpp)
    endif()
#    message(STATUS "TEST_FILES: ${TEST_FILES}")
    add_executable(${TEST_TARGET} ${TEST_FILES})
    target_link_libraries(${TEST_TARGET} PRIVATE ${TEST_LINK_LIBRARIES})
    _link_if_is_library(${TEST_TARGET} ${PROJECT_NAME})
    _link_if_is_library(${TEST_TARGET} ${PROJECT_NAME}-lib)

    # create project-test group
    if (NOT TARGET ${PROJECT_NAME}-tests)
        add_custom_target(${PROJECT_NAME}-tests)
    endif()
    add_dependencies(${PROJECT_NAME}-tests ${TEST_TARGET})

    if (TEST_XMOC)
        message(STATUS "Mocing: ${TEST_TARGET}")
        xdev_moc_target(${TEST_TARGET} "")
    endif()
    if (TEST_XRES_DIRS)
        foreach(dir ${TEST_XRES_DIRS})
            xdev_resources(${TEST_TARGET} ${dir})
        endforeach()
    endif()
    if (NOT TEST_WORKING_DIRECTORY)
        set(TEST_WORKING_DIRECTORY ${${PROJECT_NAME}_BINARY_DIR})
    endif()
    target_link_libraries(${TEST_TARGET} PRIVATE CONAN_PKG::gtest Threads::Threads)
    if (NOT ${${_no_test_var_name}})
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_TARGET})
        #gtest_discover_tests(${TEST_TARGET})
    else()
        message(STATUS "test ${TEST_TARGET} disabled")
    endif()
    if(TARGET ${PROJECT_NAME})
        get_target_property(_test_folder ${PROJECT_NAME} FOLDER)
        if(_test_folder)
            set(_test_folder "${_test_folder}/${PROJECT_NAME}-tests")
        endif()
    endif()
    set_target_properties(${TEST_TARGET} PROPERTIES FOLDER ${_test_folder})
endmacro()


macro(xdev_add_simple_tests)
    file(GLOB _files *.cpp)
    foreach(_file ${_files})
        get_filename_component(_name ${_file} NAME_WE)
        xdev_add_test(NAME ${_name})
    endforeach()
endmacro()

function(xdev_library _target)
    set(options MOC SHARED NO_INSTALL)
    set(oneValueArgs ALIAS CXX_STANDARD FOLDER)
    set(multiValueArgs
        SOURCES
        PUBLIC_HEADERS
        PUBLIC_LIBRARIES
        PUBLIC_INCLUDE_DIRS
        RESOURCES_DIRS
    )
    cmake_parse_arguments(LIB
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(LIB_SHARED)
        set(_lib_type SHARED)
    endif()

    add_library(${_target} ${_lib_type} ${LIB_SOURCES} ${LIB_PUBLIC_HEADERS})

    if(LIB_FOLDER)
        set_target_properties(${_target} PROPERTIES FOLDER ${LIB_FOLDER})
        set(_generated_include_dirs ${PROJECT_BINARY_DIR}/include/${LIB_FOLDER})
    else()
        set(_generated_include_dirs ${PROJECT_BINARY_DIR}/include)
    endif()

    generate_export_header(${_target} EXPORT_FILE_NAME ${_generated_include_dirs}/${_target}-export.hpp)
    list(APPEND LIB_PUBLIC_HEADERS ${_generated_include_dirs}/${_target}-export.hpp)
    target_sources(${_target} PUBLIC ${_generated_include_dirs}/${_target}-export.hpp)
    target_include_directories(${_target} PUBLIC include inline ${PROJECT_BINARY_DIR}/include ${LIB_PUBLIC_INCLUDE_DIRS})
    target_link_libraries(${_target} PUBLIC ${LIB_PUBLIC_LIBRARIES})

    if(LIB_ALIAS)
        add_library(${LIB_ALIAS} ALIAS ${_target})
    endif()

    if(LIB_CXX_STANDARD)
        set_target_properties(${_target} PROPERTIES CXX_STANDARD ${LIB_CXX_STANDARD})
        set_target_properties(${_target} PROPERTIES CXX_STANDARD_REQUIRED ON)
    endif()


    if (LIB_MOC)
        xdev_moc_target(${_target} "")
    endif()

    if (LIB_RESOURCES_DIRS)
        foreach(_dir ${LIB_RESOURCES_DIRS})
            xdev_resources(${_target} ${_dir})
        endforeach()
    endif()

    if (EXISTS ${${_target}_SOURCE_DIR}/tests)
        add_subdirectory(${${_target}_SOURCE_DIR}/tests)
    endif()

    if(LIB_PUBLIC_HEADERS)
        set_target_properties(${_target} PROPERTIES PUBLIC_HEADER "${LIB_PUBLIC_HEADERS}")
    endif()

    if(NOT LIB_NO_INSTALL)
        install(TARGETS ${_target}
            LIBRARY
              DESTINATION lib
              COMPONENT Libraries
              NAMELINK_COMPONENT Development
            PUBLIC_HEADER
              DESTINATION include/${LIB_FOLDER}
              COMPONENT Development
        )
    endif()
endfunction()
