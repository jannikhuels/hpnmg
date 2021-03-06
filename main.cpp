#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <signal.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>

#include "util/logging/Logging.h"
#include "modelchecker/RegionModelChecker.h"
#include "ParseHybridPetrinet.h"
#include "ReadHybridPetrinet.h"
#include "PLTWriter.h"
#include "ReadFormula.h"
#include "util/statistics/Statistics.h"

bool fileExists(const std::string &filename) {
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1) {
        return true;
    }
    return false;
}

namespace po = boost::program_options;

int process_command_line(int argc, char **argv, std::string& model, std::string& formula, double &maxtime, double &checktime, string& resultfile, std::vector<string> &appendix, string &fileLogLevel, string &coutLogLevel) {
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
                ("file-loglevel", po::value<std::string>(&fileLogLevel), "Log level of the file output.")
                ("stdout-loglevel", po::value<std::string>(&coutLogLevel), "Log level of the stdout output.")
                ;

        if (argc == 1) {
            cout << endl << "No option was specified." << endl << endl << desc << endl;
            return 2;
        }

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 2;
        }

        po::notify(vm);

        if (vm.count("file-loglevel") && !(fileLogLevel == "ALL" || fileLogLevel == "TRACE" || fileLogLevel == "DEBUG" || fileLogLevel == "INFO" || fileLogLevel == "WARN" || fileLogLevel == "ERROR" || fileLogLevel == "FATAL")) {
            throw po::validation_error(po::validation_error::invalid_option_value, "file-loglevel");
        }

        if (vm.count("stdout-loglevel") && !(coutLogLevel == "ALL" || coutLogLevel == "TRACE" || coutLogLevel == "DEBUG" || coutLogLevel == "INFO" || coutLogLevel == "WARN" || coutLogLevel == "ERROR" || coutLogLevel == "FATAL")) {
            throw po::validation_error(po::validation_error::invalid_option_value, "cout-loglevel");
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

void handler(int s){
    PRINT_STATS()
    exit(1);
}

int main (int argc, char *argv[]) {
    std::string modelfile;
    std::string formulafile;
    std::string resultfile;
    std::string fileLogLevel;
    std::string coutLogLevel;
    std::vector<string> appendix;
    double maxtime;
    double checktime;
    int mode = process_command_line(argc, argv, modelfile, formulafile, maxtime, checktime, resultfile, appendix, fileLogLevel, coutLogLevel);

    if (mode == 2) {
        // An error occured.
        return 1;
    }
    // mode==1: Only create state space, mode==0 also perform model checking.

    // Initialize logging, may be influence by program options
    hpnmg::initializeLogging(coutLogLevel, fileLogLevel);

    // Register handler to print statistics also when CTRL-C is pressed
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // Start reading the model file.
    std::time_t startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto startTotal = std::chrono::high_resolution_clock::now();

    const auto startRead = std::chrono::high_resolution_clock::now();
    auto hpng = hpnmg::ReadHybridPetrinet{}.readHybridPetrinet(modelfile);
    const auto endRead = std::chrono::high_resolution_clock::now();
    auto readTime = std::chrono::duration_cast<std::chrono::milliseconds>(endRead - startRead).count();

    cout << "[Reading]: " << readTime << "ms" << endl;
    cout << "[Start parsing for maxtime]: " << maxtime << endl;

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

        cout << "[Start model checking for checktime]: " << checktime << endl;

        const auto checkResult = checker.satisfies(formula, checktime);
        const auto endSatisfy = std::chrono::high_resolution_clock::now();
        const auto modelCheckingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endSatisfy - startSatisfy).count();
        cout << "[Probability]: " << checkResult.first << endl;
        cout << "[Error]: " << checkResult.second << endl;
        cout << "[ModelChecking]: " << modelCheckingTime << endl;

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

    PRINT_STATS()

    resultfilestream << endl;
    resultfilestream.close();

    return 0;
}