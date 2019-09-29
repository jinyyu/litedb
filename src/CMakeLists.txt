

set(SRC
    elog.cpp
    mcxt.cpp
    pq.cpp
    session.cpp)

add_library(litesql ${SRC})
target_link_libraries(litesql ${LIBS})

add_executable(litemaster master.cpp)
target_link_libraries(litemaster ${LIBS} litesql)

