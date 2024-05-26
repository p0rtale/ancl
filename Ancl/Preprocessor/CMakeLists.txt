FLEX_TARGET(PreLexer ${CMAKE_SOURCE_DIR}/Ancl/Preprocessor/Lexer/Lexer.l
                     ${CMAKE_SOURCE_DIR}/Ancl/Preprocessor/Lexer/Lexer.cpp)

file(GLOB_RECURSE PREPROCESSOR_SOURCES *.cpp)

file(GLOB_RECURSE LOGGER_SOURCES ${CMAKE_SOURCE_DIR}/Ancl/Logger/*.cpp)
add_library(preprocessor SHARED ${PREPROCESSOR_SOURCES} ${FLEX_PreLexer_OUTPUTS} ${LOGGER_SOURCES})
target_link_libraries(preprocessor PRIVATE spdlog::spdlog)

target_include_directories(preprocessor PUBLIC ${LIB_INCLUDE_PATH})
