
BISON_TARGET(parser parser/gram.y ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp)
FLEX_TARGET(scanner parser/scan.l ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(bin_src
        bin/initdb.cpp
        bin/server.cpp
        bin/session.cpp
        )
set(catalog_src
        catalog/catalog.cpp
        catalog/sys_attribute.cpp
        catalog/sys_class.cpp
        catalog/sys_index.cpp
        )
set(command_src
        command/indexcmd.cpp)

set(node_src
        nodes/execnodes.cpp
        nodes/parsenodes.cpp
        nodes/value.cpp
        )
set(parser_src
        parser/analyze.cpp
        parser/keywords.cpp
        parser/parse_clause.cpp
        parser/parser.cpp
        )

set(storage_src
        storage/databasemdb.cpp
        storage/index.cpp
        storage/relation.cpp
        storage/scan_key.cpp
        storage/tuple.cpp
        )
set(utils_src
        utils/compare.cpp
        utils/env.cpp
        utils/elog.cpp
        utils/list.cpp
        utils/misc.cpp
        utils/portal.cpp
        utils/pq.cpp
        utils/vector.cpp
        )

set(litedb_src
        ${BISON_parser_OUTPUTS}
        ${FLEX_scanner_OUTPUTS}
        ${bin_src}
        ${catalog_src}
        ${command_src}
        ${node_src}
        ${parser_src}
        ${storage_src}
        ${utils_src}
        )

add_library(litedb++ ${litedb_src})
target_link_libraries(litedb++ ${LIBS})

set(LITEDB_LIB litedb++)

add_executable(litedb bin/litedb.cpp)
target_link_libraries(litedb ${LIBS} litedb++)

