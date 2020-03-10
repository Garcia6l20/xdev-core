cmake_minimum_required(VERSION 3.12)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(XDEV_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "xdev cmake module path")

include(FetchContent)
include(CTest)


option(XDEV_MELTINGPOT_DEV "Use MetlingPot as submodule" OFF)
if(XDEV_MELTINGPOT_DEV)
  # as MP developer
  include(${CMAKE_CURRENT_LIST_DIR}/MeltingPot/MeltingPot.cmake)
else()
  # as MP user
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/MeltingPot.cmake")
    message(STATUS "Downloading MeltingPot.cmake from https://github.com/Garcia6l20/MeltingPot")
    file(DOWNLOAD "https://raw.githubusercontent.com/Garcia6l20/MeltingPot/v0.1.x/dist/MeltingPot.cmake" "${CMAKE_BINARY_DIR}/MeltingPot.cmake")
  endif()
  include(${CMAKE_BINARY_DIR}/MeltingPot.cmake)
endif()
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CTEST_OUTPUT_ON_FAILURE ON)

option(XDEV_UNIT_TESTING "Enable unit testing" ON)

if(NOT TARGET RERUN_CMAKE)
    add_custom_target(RERUN_CMAKE COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR})
    set_target_properties(RERUN_CMAKE PROPERTIES FOLDER CMakePredefinedTargets)
endif()

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
    # message("Moc[${target}] files: ${full_path_header_files}")
    if (NOT TARGET xdev-moc)
        find_program(MOC_EXE_OR_TARGET xdev-moc)
        set(_deps ${header_files})
    else()
        set(MOC_EXE_OR_TARGET xdev-moc)
        set(_deps "xdev-moc;${header_files}")
    endif()
    add_custom_command(
        OUTPUT ${generated_files}
        DEPENDS ${_deps}
        COMMAND ${MOC_EXE_OR_TARGET} ARGS ${target} ${full_path_header_files}
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
    if (NOT TARGET xdev-rc)
        find_program(RC_EXE_OR_TARGET xdev-rc)
        set(_deps ${resource_files})
    else()
        set(RC_EXE_OR_TARGET xdev-rc)
        set(_deps "xdev-rc;${resource_files}")
    endif()
    add_custom_command(
        OUTPUT ${generated_files}
        DEPENDS ${_deps}
        COMMAND ${RC_EXE_OR_TARGET} ARGS ${target} ${CMAKE_CURRENT_SOURCE_DIR}/${directory}
        COMMENT "Generating resources for ${target} (${output_dir})"
        WORKING_DIRECTORY ${output_dir}
    )
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

# override melt parsing
set(_MELT_TARGET_PARSE_OPTIONS ${_MELT_TARGET_PARSE_OPTIONS}
  MOC
)
set(_MELT_TARGET_PARSE_MULTI_VALUE_ARGS ${_MELT_TARGET_PARSE_MULTI_VALUE_ARGS}
  RESOURCES_DIRS
)

macro(xdev_add_test _name)

    melt_add_test(${_name} ${ARGN})

    if (MELT_ARGS_MOC)
        message(STATUS "Mocing: ${_target}")
        xdev_moc_target(${_target} "")
    endif()

    if (MELT_ARGS_RESOURCES_DIRS)
        foreach(dir ${MELT_ARGS_RESOURCES_DIRS})
            xdev_resources(${_target} ${dir})
        endforeach()
    endif()

endmacro()


macro(xdev_discover_tests)
    file(GLOB _files test-*.cpp)
    foreach(_file ${_files})
        get_filename_component(_name ${_file} NAME_WE)
        string(REGEX REPLACE "test-(.*)" [[\1]] _name ${_name})
        xdev_add_test(${_name})
    endforeach()
endmacro()

function(xdev_library _target)

    melt_library(${_target} ${ARGN})

    if (MELT_ARGS_MOC)
        xdev_moc_target(${_target} "")
    endif()

    if (MELT_ARGS_RESOURCES_DIRS)
        foreach(_dir ${MELT_ARGS_RESOURCES_DIRS})
            xdev_resources(${_target} ${_dir})
        endforeach()
    endif()
endfunction()
