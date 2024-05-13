#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "CLexer.h"
#include "CParser.h"

#include <Ancl/Visitor/BuildAstVisitor.hpp>
#include <Ancl/Grammar/AST/ASTProgram.hpp>

#include <Ancl/Visitor/AstDotVisitor.hpp>
#include <Ancl/Visitor/IRGenAstVisitor.hpp>

#include <Ancl/AnclIR/IRProgram.hpp>

using namespace anclgrammar;
using namespace antlr4;

using namespace ir;


int main(int argc, const char** argv) {
    std::ifstream file{"test.c"};
    ANTLRInputStream input(file);
    CLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    std::vector<Token*> lineTokens;

    tokens.fill();
    for (Token* token : tokens.getTokens()) {
        if (token->getChannel() == CLexer::LINE) {
            lineTokens.push_back(token);
        }
        std::cout << token->toString() << std::endl;
    }

    CParser parser(&tokens);
    auto* tree = parser.translationUnit();

    std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

    ASTProgram astProgram;
    BuildAstVisitor buildVisitor(astProgram);
    buildVisitor.setLineTokens(lineTokens);
    buildVisitor.visitTranslationUnit(tree);

    AstDotVisitor dotVisitor("ast.dot");
    dotVisitor.Visit(*astProgram.GetTranslationUnit());

    // IRProgram irProgram;
    // IRGenAstVisitor irGenVisitor(irProgram);
    // irGenVisitor.Run(astProgram);

    return EXIT_SUCCESS;
}
