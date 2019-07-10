#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "modelchecker/RegionModelChecker.h"
#include "ParseHybridPetrinet.h"
#include "ReadHybridPetrinet.h"
#include "PLTWriter.h"

int main (int argc, char *argv[]) {
    using namespace hpnmg;

    if (argc != 4)
        throw std::runtime_error("provide (only) model file path, max time and check time.");
    const auto modelFile = std::string(argv[1]);
    const auto maxTime = [&argv]() {
        errno = 0;
        auto result = std::strtod(argv[2], nullptr);
        if (errno != 0)
            throw std::invalid_argument(std::string("bad max time: ") + std::string(argv[2]));

        return result;
    }();
    const auto checkTime = [&argv]() {
        errno = 0;
        auto result = std::strtod(argv[3], nullptr);
        if (errno != 0)
            throw std::invalid_argument(std::string("bad check time: ") + std::string(argv[3]));

        return result;
    }();

    const auto startTotal = std::chrono::high_resolution_clock::now();

    const auto startRead = std::chrono::high_resolution_clock::now();
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet(modelFile);
    const auto endRead = std::chrono::high_resolution_clock::now();
    std::cout << "Reading took " << std::chrono::duration_cast<std::chrono::milliseconds>(endRead - startRead).count() << "ms." << std::endl;

    const auto startParse = std::chrono::high_resolution_clock::now();
    const auto plt = ParseHybridPetrinet{}.parseHybridPetrinet(hpng, maxTime);
    const auto endParse = std::chrono::high_resolution_clock::now();
    std::cout << "Parsing took " << std::chrono::duration_cast<std::chrono::milliseconds>(endParse - startParse).count() << "ms." << std::endl;

    const auto startWrite = std::chrono::high_resolution_clock::now();
    PLTWriter{}.writePLT(plt, maxTime);
    const auto endWrite = std::chrono::high_resolution_clock::now();
    std::cout << "Writing took " << std::chrono::duration_cast<std::chrono::milliseconds>(endWrite - startWrite).count() << "ms." << std::endl;

    const auto startChecker = std::chrono::high_resolution_clock::now();
    auto checker = RegionModelChecker(*hpng, maxTime);
    const auto endChecker = std::chrono::high_resolution_clock::now();
    std::cout << "Initializing the checker took " << std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count() << "ms." << std::endl;

    const auto startSatisfy = std::chrono::high_resolution_clock::now();
    const auto checkResult = checker.satisfies(
//        Formula(std::make_shared<Conjunction>(
//            Formula(std::make_shared<ContinuousAtomicProperty>("H", 100)),
//            Formula(std::make_shared<Negation>(Formula(std::make_shared<ContinuousAtomicProperty>("H", 100))))
//        )),
        Formula(std::make_shared<DiscreteAtomicProperty>("P1_On", 1)),
        checkTime
    );
    const auto endSatisfy = std::chrono::high_resolution_clock::now();
    std::cout << "Checking the formula took " << std::chrono::duration_cast<std::chrono::milliseconds>(endSatisfy - startSatisfy).count() << "ms." << std::endl;
    std::cout << "The result (probability, error): (" << checkResult.first << ", " << checkResult.second << ")." << std::endl;

//    const auto startSerialize = std::chrono::high_resolution_clock::now();
//    // create and open a character archive for output
//    std::ofstream ofs("checker_archive.txt");
//    boost::archive::text_oarchive{ofs} << checker;
//    const auto endSerialize = std::chrono::high_resolution_clock::now();
//    std::cout << "Serializing took " << std::chrono::duration_cast<std::chrono::milliseconds>(endParse - startParse).count() << "ms." << std::endl;

    const auto endTotal = std::chrono::high_resolution_clock::now();
    std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTotal - startTotal).count() << "ms."
        << "(" << std::chrono::duration_cast<std::chrono::seconds>(endTotal - startTotal).count() << "s)" << std::endl;
}