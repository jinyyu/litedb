
set(test_LIBS
        litedb++
        ${gtest_LIBS}
        )

add_executable(test_tuple test_tuple.cpp)
target_link_libraries(test_tuple ${test_LIBS})