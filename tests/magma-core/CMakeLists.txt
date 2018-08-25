set(SOURCES core-test.cpp)
CREATE_UNIT_TEST(core-test magma: "${SOURCES}")
target_link_libraries(core-test magma::core)
