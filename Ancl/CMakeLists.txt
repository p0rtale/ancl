get_filename_component(LIB_INCLUDE_PATH ".." ABSOLUTE)

add_subdirectory(Grammar)

file(GLOB_RECURSE SOURCES Grammar/AST/*.cpp Visitor/*.cpp Driver/*.cpp
                          AnclIR/*.cpp DataLayout/*.cpp CodeGen/*.cpp)
file(GLOB_RECURSE LOGGER_SOURCES Logger/*.cpp)

add_subdirectory(Preprocessor)

add_library(ancl ${SOURCES} ${LOGGER_SOURCES})
target_include_directories(ancl PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(ancl PUBLIC antlrgrammar PUBLIC preprocessor PRIVATE spdlog::spdlog)
