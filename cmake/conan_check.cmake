if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)

    message(STATUS "conan_paths not found, runing it for you !")

    find_program(CONAN_EXE conan)
    if(NOT CONAN_EXE)
        message(FATAL_ERROR "Unable to find conan executable, please install it !")
    endif()

    string(STRIP ${CMAKE_CXX_COMPILER_VERSION} XDEV_CXX_COMPILER_VERSION)
    string(REGEX REPLACE "^([0-9]+).*$" "\\1"
           XDEV_CXX_COMPILER_VERSION_MAJOR ${XDEV_CXX_COMPILER_VERSION})
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2"
           XDEV_CXX_COMPILER_VERSION_MINOR ${XDEV_CXX_COMPILER_VERSION})
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3"
           XDEV_CXX_COMPILER_VERSION_PATCH ${XDEV_CXX_COMPILER_VERSION})

    set(XDEV_CONAN_CXX_COMPILER_VERSION ${XDEV_CXX_COMPILER_VERSION_MAJOR}.${XDEV_CXX_COMPILER_VERSION_MINOR})
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(_compiler_version_setting -s compiler.version=${XDEV_CONAN_CXX_COMPILER_VERSION})
    endif()

    execute_process(COMMAND ${CONAN_EXE} install ${CMAKE_SOURCE_DIR}
            -s compiler.cppstd=${CMAKE_CXX_STANDARD}
            ${_compiler_version_setting}
            -s build_type=${CMAKE_BUILD_TYPE}
            --build=missing
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
