
BISON_TARGET(parser parser/gram.y ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp)
FLEX_TARGET(scanner parser/scan.l ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

set(SRC
        ${BISON_parser_OUTPUTS}
        ${FLEX_scanner_OUTPUTS}
        elog.cpp
        mcxt.cpp
        parser/parser.cpp
        pq.cpp
        session.cpp parser/parser.cpp)

add_library(litesql ${SRC})
target_link_libraries(litesql ${LIBS})

add_executable(litemaster master.cpp)
target_link_libraries(litemaster ${LIBS} litesql)

