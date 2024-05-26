#include <iostream>
#include <fstream>

#include <Ancl/Base.hpp>
#include <Ancl/Preprocessor/Preprocessor.hpp>

using namespace preproc;


int main() {
    ancl::Logger::init();
    Preprocessor preproc;

    std::string filename = "main.c";
    std::string preprocessed = preproc.Run(filename);

    std::string outputFilename = "premain.c";
    std::ofstream stream(outputFilename);
    stream << preprocessed;
    stream.close();

    return EXIT_SUCCESS;
}
