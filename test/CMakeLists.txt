
set(test_LIBS
        litedb++
        ${gtest_LIBS}
        )

add_executable(test_tuple test_tuple.cpp)
target_link_libraries(test_tuple ${test_LIBS})

add_executable(test_database test_database.cpp)
target_link_libraries(test_database ${test_LIBS})

add_executable(test_relation test_relation.cpp)
target_link_libraries(test_relation ${test_LIBS})