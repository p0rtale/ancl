set(ANTLR4_WITH_STATIC_CRT OFF) # using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR_BUILD_SHARED ON) # in my case I built the shared version
set(ANTLR_TAG 4.13.1) # to specify the GitHub tag for the jar too
set(DISABLE_WARNINGS ON) # Antlr gives a few deprecation warnings for me
set(ANTLR_BUILD_CPP_TESTS OFF) # to avoid creating dist folder

get_filename_component(ANTLR_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/antlr-${ANTLR_TAG}-complete.jar" ABSOLUTE)
if(NOT EXISTS "${ANTLR_EXECUTABLE}")
    file(
        DOWNLOAD
        "https://www.antlr.org/download/antlr-${ANTLR_TAG}-complete.jar"
        "${ANTLR_EXECUTABLE}"
    )
endif()

include(FetchContent)

FetchContent_Declare(
    antlr
    GIT_REPOSITORY https://github.com/antlr/antlr4
    GIT_TAG ${ANTLR_TAG}
    SOURCE_SUBDIR "runtime/Cpp"
)
FetchContent_MakeAvailable(antlr) # populates antlr_SOURCE_DIR
include(${antlr_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake) # populates ANTLR_TARGET

antlr_target(AnclGrammarLexer  CLexer.g4  LEXER PACKAGE anclgrammar)
antlr_target(AnclGrammarParser CParser.g4 PARSER VISITOR PACKAGE anclgrammar
                                              DEPENDS_ANTLR AnclGrammarLexer
                                              COMPILE_FLAGS -lib ${ANTLR_AnclGrammarLexer_OUTPUT_DIR})

# add generated grammar to demo binary target
add_library(antlrgrammar ${ANTLR_AnclGrammarLexer_CXX_OUTPUTS}
                         ${ANTLR_AnclGrammarParser_CXX_OUTPUTS})

target_link_libraries(antlrgrammar PUBLIC antlr4_shared)
target_include_directories(antlrgrammar PUBLIC "${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src" ${ANTLR_AnclGrammarLexer_OUTPUT_DIR}
                                                                                             ${ANTLR_AnclGrammarParser_OUTPUT_DIR})

# if you want to copy them to a different location and haven't specified
# CMAKE_LIBRARY_OUTPUT_DIRECTORY
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY
# CMAKE_RUNTIME_OUTPUT_DIRECTORY
# set(ANTLR_RUNTIME_LIBRARIES $<TARGET_FILE:antlr4_shared> $<TARGET_LINKER_FILE:antlr4_shared>)
