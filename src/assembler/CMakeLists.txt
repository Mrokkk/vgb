find_package(BISON)

BISON_TARGET(parser
    parser.y
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/parser.hpp
    VERBOSE REPORT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/parser.out)

add_library(assembler OBJECT

    argument.cpp
    assembler.cpp
    directives.cpp
    error.cpp
    expression.cpp
    instruction.cpp
    label.cpp
    lexer.cpp

    ${BISON_parser_OUTPUTS}
)

set_source_files_properties(${BISON_parser_OUTPUTS} PROPERTIES COMPILE_OPTIONS -fno-exceptions)

target_include_directories(assembler PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(assembler PUBLIC fmt)
# target_compile_definitions(assembler PRIVATE -DYYDEBUG=1)
