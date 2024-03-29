#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "CLexer.h"
#include "CParser.h"

#include <Ancl/Visitor/BuildAstVisitor.hpp>
#include <Ancl/Grammar/AST/Program.hpp>


using namespace anclgrammar;
using namespace antlr4;


int main(int argc, const char** argv) {
    std::ifstream file{"test.c"};
    ANTLRInputStream input(file);
    CLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
      std::cout << token->toString() << std::endl;
    }

    CParser parser(&tokens);
    auto* tree = parser.translationUnit();

    std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

    // Program program;
    // BuildAstVisitor visitor(program);

    // visitor.visitTranslationUnit(tree);

    return 0;
}
