file(GLOB_RECURSE HEADERS magma/*.hpp)
file(GLOB_RECURSE SOURCES magma/*.cpp)
set(MODULE_SOURCES ${HEADERS} ${SOURCES})
CREATE_MODULE(magma::core "${MODULE_SOURCES}" ${CMAKE_CURRENT_SOURCE_DIR}/magma "magma")

find_package(claws CONFIG REQUIRED)

target_link_libraries(core INTERFACE
  claws::utils
  claws::container
  claws::iterator
)

AUTO_TARGETS_MODULE_INSTALL(core)
