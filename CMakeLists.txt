if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and call cmake from there")
endif ()

cmake_minimum_required(VERSION 3.9)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

##! Project
project(magma)

##! Prerequisites CTEST
enable_testing()

##! Project options
option(MAGMA_BUILD_TESTS "Build magma tests" OFF)
option(MAGMA_BUILD_EXAMPLES "Build magma examples" OFF)
option(IDE_BUILD "Workaround for header-only libraries, put it to ON if you use CLION" OFF)

##! CMake Path
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(library)

##! Project modules
add_subdirectory(modules)

##! Project tests
if (MAGMA_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

##! Project examples
if (MAGMA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

##! Main library
add_library(magma INTERFACE)

if (NOT DEFINED ENV{CLAWS_ROOT_DEV})
    message(STATUS "using insalled version of claws")
    find_package(claws CONFIG REQUIRED)
else ()
    message(STATUS "using development version of claws")
    add_subdirectory($ENV{CLAWS_ROOT_DEV} build)
endif ()

target_link_libraries(magma INTERFACE
  magma::core
)

if (NOT IDE_BUILD)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    install(TARGETS
            magma
            EXPORT magma-targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            )

    install(DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/modules/magma
            DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp"
            )

    install(EXPORT magma-targets
            FILE magma-targets.cmake
            NAMESPACE magma::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/magma
            )

    configure_package_config_file(
            "${PROJECT_SOURCE_DIR}/cmake/magma-config.cmake.in"
            "${PROJECT_BINARY_DIR}/magma-config.cmake"
            INSTALL_DESTINATION lib/cmake/magma
    )

    install(FILES
            "${PROJECT_BINARY_DIR}/magma-config.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/magma)
endif ()
