
BISON_TARGET(parser parser/gram.y ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp)
FLEX_TARGET(scanner parser/scan.l ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SRC
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
    exec/session.cpp
    parser/analyze.cpp
    parser/keywords.cpp
    parser/nodes.cpp
    parser/parser.cpp
    utils/env.cpp
    utils/elog.cpp
    utils/portal.cpp
    utils/pq.cpp
    )

add_library(litedb++ ${SRC})
target_link_libraries(litedb++ ${LIBS})

add_executable(litedb exec/main.cpp)
target_link_libraries(litedb ${LIBS} litedb++)

