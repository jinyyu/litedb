
BISON_TARGET(parser parser/gram.y ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp)
FLEX_TARGET(scanner parser/scan.l ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SRC
        ${BISON_parser_OUTPUTS}
        ${FLEX_scanner_OUTPUTS}
        btree_rb.cpp
        elog.cpp
        mcxt.cpp
        parser/keywords.cpp
        parser/nodes.cpp
        parser/parser.cpp
        pq.cpp
        session.cpp parser/parser.cpp)

add_library(litesql ${SRC})
target_link_libraries(litesql ${LIBS})

add_executable(dbMaster master.cpp)
target_link_libraries(dbMaster ${LIBS} litesql)

