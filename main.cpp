#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>

#include "modelchecker/RegionModelChecker.h"
#include "ParseHybridPetrinet.h"
#include "ReadHybridPetrinet.h"
#include "PLTWriter.h"
#include "ReadFormula.h"

bool fileExists(const std::string &filename) {
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1) {
        return true;
    }
    return false;
}

namespace po = boost::program_options;

int process_command_line(int argc, char **argv, std::string& model, std::string& formula, double &maxtime, double &checktime, string& resultfile, std::vector<string> &appendix) {
    try {
        po::options_description desc("Allowed options are");
        desc.add_options()
                ("help,h", "Produce help message.")
                ("stateonly,s", "Only create the state space.")
                ("model", po::value<std::string>(&model)->required(), "Path to the model .xml file.")
                ("formula,f", po::value<std::string>(&formula)->default_value(""), "Path to the formula .xml file.")
                ("additional,a", po::value<std::vector<std::string>>(&appendix)->multitoken(), "Additional parameters appended to the output file.")
                ("maxtime", po::value<double>(&maxtime)->required(), "Maxtime of state space creation.")
                ("checktime,c", po::value<double>(&checktime)->default_value(0), "Checktime of model checking.")
                ("result,r", po::value<std::string>(&resultfile)->default_value("result.dat"), "Path to the result file.")
                ;

        if (argc == 1) {
            cout << endl << "No option was specified." << endl << endl << desc << endl;
            return 1;
        }

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 1;
        }

        if (vm.count("stateonly")) {
            return 1;
        }
        return 0;
    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
    catch(...)
    {
        std::cerr << "Unknown error!" << "\n";
        return 2;
    }
}

int main (int argc, char *argv[]) {
    std::string modelfile;
    std::string formulafile;
    std::string resultfile;
    std::vector<string> appendix;
    double maxtime;
    double checktime;
    int mode = process_command_line(argc, argv, modelfile, formulafile, maxtime, checktime, resultfile, appendix);

    if (mode == 2) {
        // An error occured.
        return 1;
    }
    // mode==1: Only create state space, mode==0 also perform model checking.

    // Start reading the model file.
    std::time_t startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto startTotal = std::chrono::high_resolution_clock::now();

    const auto startRead = std::chrono::high_resolution_clock::now();
    auto hpng = hpnmg::ReadHybridPetrinet{}.readHybridPetrinet(modelfile);
    const auto endRead = std::chrono::high_resolution_clock::now();
    auto readTime = std::chrono::duration_cast<std::chrono::milliseconds>(endRead - startRead).count();

    cout << "[Reading]: " << readTime << "ms" << endl;

    const auto startChecker = std::chrono::high_resolution_clock::now();
    auto checker = hpnmg::RegionModelChecker(*hpng, maxtime);
    const auto endChecker = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();

    cout << "[Parsing]: " << initTime << "ms" << endl;

    bool writeheader = false;
    if (!fileExists(resultfile)) {
        writeheader = true;
    }

    fstream resultfilestream {resultfile, resultfilestream.out | resultfilestream.app};

    if (writeheader) {
        resultfilestream << "# [Start time]\t" <<
                            "[Model file]\t" <<
                            "[Read duration (ms)]\t" <<
                            "[Parse duration (ms)]\t" <<
                            "[Model checking duration (ms)]\t" <<
                            "[Probability]\t[Error]" << endl <<
                            "start\tmodel\tread\tparse\tcheck\tprob\terr" << endl;
    }

    //Remove line break from startTimeString
    std::string startTimeString(std::ctime(&startTime));
    startTimeString = startTimeString.substr(0, startTimeString.length()-1);
    resultfilestream << startTimeString  << "\t" << modelfile << "\t" << readTime << "\t" << initTime;
    double prob = 0;
    double err = 0;
    if (mode == 1) {
        cout << "[Mode]: State space creation only was chosen." << endl;
        cout << "[Results]: " << resultfile << endl;
    } else {
        const auto startSatisfy = std::chrono::high_resolution_clock::now();

        if (formulafile.size() == 0) {
            std::cerr << "Please specify a formula file (-f) to perform model checking." << endl;
            resultfilestream << "\t" << 0 << "\t" << prob << "\t" << err;
            resultfilestream << endl;
            resultfilestream.close();
            return 1;
        }

        hpnmg::Formula formula = hpnmg::ReadFormula{}.readFormula(formulafile);

        const auto checkResult = checker.satisfies(formula, checktime);
        const auto endSatisfy = std::chrono::high_resolution_clock::now();
        const auto modelCheckingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endSatisfy - startSatisfy).count();
        cout << "[Probability]: " << checkResult.first << endl;
        cout << "[Error]: " << checkResult.second << endl;
        cout << "[ModelChecking]: " << resultfile << endl;

        prob = checkResult.first;
        err = checkResult.second;
    }

    const auto endTotal = std::chrono::high_resolution_clock::now();
    const auto totalTime= std::chrono::duration_cast<std::chrono::milliseconds>(endTotal - startTotal).count();
    cout << "[Complete]: " << totalTime  << endl;

    resultfilestream << "\t" << totalTime << "\t" << prob << "\t" << err;

    // Write appendix to resultfile
    for (string s : appendix) {
        resultfilestream << "\t" << s;
    }

    resultfilestream << endl;
    resultfilestream.close();

    return 0;
}

/*int main (int argc, char *argv[]) {

    using namespace hpnmg;

    fstream resultfile {RESULT_FILE, resultfile.out | resultfile.app};

    if (argc == 1)
        throw std::runtime_error("no parameters specified.");

    std::map<std::string,std::string> parameterIndexMap;
    parameterIndexMap[MODE_PARAMETER] = argv[1];

    const auto mode = [&argv, argc, &parameterIndexMap]() {
        errno = 0;
        auto result = std::string(parameterIndexMap.find(MODE_PARAMETER)->second);
        if (result == "-s") {
            if (argc<4) {
                throw std::invalid_argument("invalid number of parameters specified.");
            }
            parameterIndexMap[MODEL_PARAMETER] = argv[2];
            parameterIndexMap[MAXTIME_PARAMETER] = argv[3];
            return statespaceonly;
        }
        if (result == "-m") {
            if (argc<6) {
                throw std::invalid_argument("invalid number of parameters specified.");
            }
            parameterIndexMap[MODEL_PARAMETER] = argv[2];
            parameterIndexMap[FORMULA_PARAMETER] = argv[3];
            parameterIndexMap[CHECKTIME_PARAMETER] = argv[4];
            parameterIndexMap[MAXTIME_PARAMETER] = argv[5];
            return modelchecking;
        }

        throw std::invalid_argument("unknown mode specified.");
    }();



    const auto modelFile = std::string(argv[parameterIndexMap.find(MODEL_PARAMETER)->second]);

    const auto maxTime = [&argv,mode,&parameterIndexMap]() {
        errno = 0;
        auto result = std::strtod(argv[parameterIndexMap.find(MAXTIME_PARAMETER)->second], nullptr);
        if (errno != 0) {
            std::string val = std::string(argv[parameterIndexMap.find(MAXTIME_PARAMETER)->second]);
            throw std::invalid_argument(std::string("bad max time: ") + val);
        }

        return result;
    }();

    auto checkTime = 0;
    if (mode == modelchecking) {
        checkTime = [&argv, &parameterIndexMap]() {
            errno = 0;
            auto result = std::strtod(argv[parameterIndexMap.find(CHECKTIME_PARAMETER)->second], nullptr);
            if (errno != 0)
                throw std::invalid_argument(std::string("bad check time: ") + std::string(argv[parameterIndexMap.find(CHECKTIME_PARAMETER)->second]));

            return result;
        }();
    }

    const auto startTotal = std::chrono::high_resolution_clock::now();

    const auto startRead = std::chrono::high_resolution_clock::now();
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet(modelFile);

    const auto endRead = std::chrono::high_resolution_clock::now();
    auto readingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endRead - startRead).count();
    std::cout << "Reading took " << readingTime << "ms." << std::endl;

    const auto startChecker = std::chrono::high_resolution_clock::now();
    auto checker = RegionModelChecker(*hpng, maxTime);
    const auto endChecker = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endChecker - startChecker).count();
    std::cout << "Initializing the checker took " << initTime << "ms." << std::endl;

    if (mode == statespaceonly) {

        measurefile << readingTime << "\t" << initTime;
        // Append additional parameters to the output file
        if (argc > 4) {
            for (int i = 4; i < argc; i++) {
                measurefile << "\t" << std::string(argv[i]);
            }
        }
        measurefile << endl;

        //    const auto startWrite = std::chrono::high_resolution_clock::now();
//    auto plt = ParseHybridPetrinet{}.parseHybridPetrinet(hpng, maxTime);
//    plt->updateRegions();
//    PLTWriter{}.writePLT(plt, maxTime);
//    const auto endWrite = std::chrono::high_resolution_clock::now();
//    std::cout << "Parsing and writing took " << std::chrono::duration_cast<std::chrono::milliseconds>(endWrite - startWrite).count() << "ms." << std::endl;
//
//    auto ws = std::vector<ParametricLocationTree::Node>{plt->getRootNode()};
//    while (!ws.empty()) {
//        const auto node = ParametricLocationTree::Node(ws.back());
//        ws.pop_back();
//        auto childNodes = plt->getChildNodes(node);
//        ws.insert(ws.end(), childNodes.begin(), childNodes.end());
//
//        std::cout << "node: " << node.getNodeID() << STDPolytope<mpq_class>(STDPolytope<mpq_class>(node.getRegion()).timeSlice(checkTime)) << std::endl;
//    }

        return 0;
    }

    const auto startSatisfy = std::chrono::high_resolution_clock::now();

    const auto formulaFile = std::string(argv[3]);
    Formula formula = ReadFormula{}.readFormula(formulaFile);

    const auto checkResult = checker.satisfies(formula, checkTime);
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

    resultfile << checkTime << "\t" << checkResult.first << "\t" << checkResult.second << "\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endTotal - startTotal).count();

    if (argc > 5) {
        for (int i = 4; i < argc; i++) {
            resultfile << "\t" << std::string(argv[i]);
        }
    }
    resultfile << endl;

    resultfile.close();
    return 0;
}*/