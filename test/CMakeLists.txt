
set(test_LIBS
        litedb++
        ${gtest_LIBS}
        )

enable_testing()

find_package(GTest REQUIRED)

add_executable(test_tuple test_tuple.cpp)
target_link_libraries(test_tuple ${test_LIBS})
gtest_add_tests(TARGET test_tuple)

add_executable(test_database test_database.cpp)
target_link_libraries(test_database ${test_LIBS})
gtest_add_tests(TARGET test_database)

add_executable(test_relation test_relation.cpp)
target_link_libraries(test_relation ${test_LIBS})
gtest_add_tests(TARGET test_relation)

add_executable(test_vector test_vector.cpp)
target_link_libraries(test_vector ${test_LIBS})
gtest_add_tests(TARGET test_vector)