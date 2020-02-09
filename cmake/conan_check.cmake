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

    execute_process(
        COMMAND ${CONAN_EXE} profile new ./conan_profile --detect
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(_conan_compiler clang)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(_conan_compiler gcc)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        set(_conan_compiler intel)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(_conan_compiler "Visual Studio")
    endif()

    execute_process(
        COMMAND ${CONAN_EXE} profile update settings.compiler=${_conan_compiler} ./conan_profile
        COMMAND ${CONAN_EXE} profile update settings.compiler.cppstd=${CMAKE_CXX_STANDARD} ./conan_profile
        COMMAND ${CONAN_EXE} profile update settings.build_type=${CMAKE_BUILD_TYPE} ./conan_profile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    set(XDEV_CONAN_CXX_COMPILER_VERSION ${XDEV_CXX_COMPILER_VERSION_MAJOR}.${XDEV_CXX_COMPILER_VERSION_MINOR})
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        execute_process(
            COMMAND ${CONAN_EXE} profile update settings.compiler.version=${XDEV_CONAN_CXX_COMPILER_VERSION} ./conan_profile
            COMMAND ${CONAN_EXE} profile update settings.compiler.libcxx=libstdc++11 ./conan_profile
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()

    execute_process(
       COMMAND cat ./conan_profile
       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    execute_process(COMMAND ${CONAN_EXE} install ${CMAKE_SOURCE_DIR}
            --profile ./conan_profile
            --build missing
            --remote xdev
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
