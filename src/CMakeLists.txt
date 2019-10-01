

set(SRC
        elog.cpp
        mcxt.cpp
        parser.cpp
        pq.cpp
        session.cpp parser.cpp)

add_library(litesql ${SRC})
target_link_libraries(litesql ${LIBS})

add_executable(litemaster master.cpp)
target_link_libraries(litemaster ${LIBS} litesql)

